#include "loaders/vulkan_loader.hpp"

#include "utils/vulkan/common.hpp"
#include "utils/vulkan/debug_messenger.hpp"

#include "volk/volk.h"
#include "GLFW/glfw3.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <algorithm>
#include <array>
#include <format>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <utility>

namespace Prism::Loaders
{
    namespace
    {
        std::vector<const char*> getRequiredInstanceExtensions()
        {
            uint32_t     glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef DEBUG
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
            extensions.push_back("VK_KHR_surface");

#ifdef PLATFORM_MAC
            extensions.push_back("VK_MVK_macos_surface");
            extensions.push_back("VK_KHR_portability_enumeration");
            extensions.push_back("VK_KHR_get_physical_device_properties2");
#endif

            return extensions;
        };

        std::vector<const char*> getRequiredDeviceExtensions()
        {
            std::vector<const char*> deviceExtensions;
            deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef PLATFORM_MAC
            deviceExtensions.push_back("VK_KHR_portability_subset");
#endif

            return deviceExtensions;
        }

        std::vector<const char*> getValidationLayers()
        {
            std::vector<const char*> validationLayers;
#ifdef DEBUG
            validationLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

            return validationLayers;
        }

        bool checkPhysicalDeviceExtensionsSupport(VkPhysicalDevice device)
        {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            auto                  deviceExtensions = getRequiredDeviceExtensions();
            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }

        Resources::VulkanDeviceAdditionalExtensions getAdditionalDeviceExtensions(VkPhysicalDevice device)
        {
            Resources::VulkanDeviceAdditionalExtensions additionalExtensions = {};

            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> additionalRTExtensions = {
                VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME};

            for (const auto& extension : availableExtensions) {
                additionalRTExtensions.erase(extension.extensionName);
            }

            if (additionalRTExtensions.empty()) {
                additionalExtensions |= Resources::VulkanDeviceAdditionalExtensions::RAYTRACING_AVAILABLE;
            }

            return additionalExtensions;
        }

        VkInstance createInstance()
        {
            VkResult result;

            VkApplicationInfo appInfo{};
            appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName   = "Prism UI";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName        = "Prism";
            appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion         = VK_API_VERSION_1_3;

            VkInstanceCreateInfo createInfo{};
            createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            auto extensions = getRequiredInstanceExtensions();

            createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();
            createInfo.flags                   = 0;

#ifdef PLATFORM_MAC
            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

            auto validationLayers          = getValidationLayers();
            createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

#ifdef DEBUG
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = Utils::Vulkan::DebugMessenger::getCreateInfo();
            createInfo.pNext                                   = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif

            VkInstance instance;
            result = vkCreateInstance(&createInfo, nullptr, &instance);
            if (result != VK_SUCCESS) {
                throw std::runtime_error(std::format("Failed to create Vulkan instance! Error code: {}\n", static_cast<int>(result)).c_str());
            }

            return instance;
        }

        VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window)
        {
            VkSurfaceKHR surface;

            if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create window surface!");
            }

            return surface;
        }

        // For now pick discrete GPU.
        VkPhysicalDevice pickPhysicalDevice(VkInstance instance)
        {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            if (deviceCount == 0) {
                throw std::runtime_error("Couldn't find GPUs with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            std::optional<VkPhysicalDevice> pickedDevice = std::nullopt;
            for (const auto& device : devices) {
                VkPhysicalDeviceProperties deviceProperties{};
                vkGetPhysicalDeviceProperties(device, &deviceProperties);

                if (!checkPhysicalDeviceExtensionsSupport(device)) {
                    continue;
                }

                if (!pickedDevice) {
                    pickedDevice = device;
                } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    pickedDevice = device;
                }
            }

            if (!pickedDevice) {
                throw std::runtime_error("Couldn't find a suitable discrete GPU!");
            }

            return pickedDevice.value();
        };

        std::pair<VkDevice, Resources::VulkanDeviceAdditionalExtensions>
        createLogicalDevice(VkPhysicalDevice physicalDevice, Utils::Vulkan::Common::QueueFamilyIndices indices)
        {
            VkDevice device;

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t>                   uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount       = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;

                queueCreateInfos.push_back(queueCreateInfo);
            }

            auto additionalDeviceExtensions = getAdditionalDeviceExtensions(physicalDevice);

            VkPhysicalDeviceFeatures deviceFeatures{};

            VkPhysicalDeviceVulkan12Features vulkan12Features{};
            vulkan12Features.sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            vulkan12Features.bufferDeviceAddress = VK_TRUE;
            // So we can have array of textures in shaders.
            vulkan12Features.runtimeDescriptorArray                    = VK_TRUE;
            vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
            vulkan12Features.descriptorBindingPartiallyBound           = VK_TRUE;

            VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
            dynamicRenderingFeatures.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
            dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
            dynamicRenderingFeatures.pNext            = &vulkan12Features;

            VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
            accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

            VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
            rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos    = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;

            std::vector<const char*> deviceExtensions = getRequiredDeviceExtensions();

            if (additionalDeviceExtensions & Resources::VulkanDeviceAdditionalExtensions::RAYTRACING_AVAILABLE) {
                deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
                deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

                accelerationStructureFeatures.accelerationStructure = VK_TRUE;
                rayTracingPipelineFeatures.rayTracingPipeline       = VK_TRUE;

                VkPhysicalDeviceSynchronization2Features synchronization2Features{};
                synchronization2Features.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
                synchronization2Features.synchronization2 = VK_TRUE;
                synchronization2Features.pNext            = &vulkan12Features;

                accelerationStructureFeatures.pNext = &synchronization2Features;
                rayTracingPipelineFeatures.pNext    = &accelerationStructureFeatures;
                dynamicRenderingFeatures.pNext      = &rayTracingPipelineFeatures;
            }

            createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            auto validationLayers = getValidationLayers();

            createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            createInfo.pNext = &dynamicRenderingFeatures;

            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create logical device!");
            }

            return std::make_pair(device, additionalDeviceExtensions);
        }

        std::pair<VkQueue, VkQueue> getQueues(VkDevice device, Utils::Vulkan::Common::QueueFamilyIndices indices)
        {
            VkQueue graphicsQueue;
            vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);

            VkQueue presentationQueue;
            vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentationQueue);

            return std::make_pair(graphicsQueue, presentationQueue);
        }
    }; // namespace

    VulkanLoader::result_type VulkanLoader::operator()(Resources::WindowResource& windowResource)
    {
        try {
            volkInitialize();

            auto instance = createInstance();
            volkLoadInstance(instance);
#ifdef DEBUG
            auto debugMessenger = std::make_unique<Utils::Vulkan::DebugMessenger>(instance);
#else
            auto debugMessenger = nullptr;
#endif

            auto surface = createSurface(instance, windowResource.GetWindow());

            auto physicalDevice = pickPhysicalDevice(instance);

            Utils::Vulkan::Common::QueueFamilyIndices indices = Utils::Vulkan::Common::findQueueFamilies(surface, physicalDevice);

            auto [device, additionalDeviceExtensions] = createLogicalDevice(physicalDevice, indices);
            volkLoadDevice(device);

            auto [graphicsQueue, presentationQueue] = getQueues(device, indices);

            auto [windowWidth, windowHeight] = windowResource.GetWindowExtent();
            VkExtent2D windowExtent          = {.width = static_cast<uint32_t>(windowWidth), .height = static_cast<uint32_t>(windowHeight)};

            std::cout << "Vulkan loaded successfully!" << std::endl;

            VmaAllocator allocator = VK_NULL_HANDLE;

            VmaAllocatorCreateInfo allocatorInfo = {};
            allocatorInfo.flags                  = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
            allocatorInfo.physicalDevice         = physicalDevice;
            allocatorInfo.device                 = device;
            allocatorInfo.instance               = instance;
            allocatorInfo.vulkanApiVersion       = VK_API_VERSION_1_3;

            // Unfortunately I think there is a bug in VMA function that does that, so we need to do it manaully.
            VmaVulkanFunctions vmaVulkanFunctions                  = {};
            vmaVulkanFunctions.vkGetInstanceProcAddr               = vkGetInstanceProcAddr;
            vmaVulkanFunctions.vkGetDeviceProcAddr                 = vkGetDeviceProcAddr;
            vmaVulkanFunctions.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
            vmaVulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
            vmaVulkanFunctions.vkAllocateMemory                    = vkAllocateMemory;
            vmaVulkanFunctions.vkFreeMemory                        = vkFreeMemory;
            vmaVulkanFunctions.vkMapMemory                         = vkMapMemory;
            vmaVulkanFunctions.vkUnmapMemory                       = vkUnmapMemory;
            vmaVulkanFunctions.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
            vmaVulkanFunctions.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
            vmaVulkanFunctions.vkBindBufferMemory                  = vkBindBufferMemory;
            vmaVulkanFunctions.vkBindImageMemory                   = vkBindImageMemory;
            vmaVulkanFunctions.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
            vmaVulkanFunctions.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
            vmaVulkanFunctions.vkCreateBuffer                      = vkCreateBuffer;
            vmaVulkanFunctions.vkDestroyBuffer                     = vkDestroyBuffer;
            vmaVulkanFunctions.vkCreateImage                       = vkCreateImage;
            vmaVulkanFunctions.vkDestroyImage                      = vkDestroyImage;
            vmaVulkanFunctions.vkCmdCopyBuffer                     = vkCmdCopyBuffer;
            // Vulkan 1.1+
            vmaVulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
            vmaVulkanFunctions.vkGetBufferMemoryRequirements2KHR       = vkGetBufferMemoryRequirements2;
            vmaVulkanFunctions.vkGetImageMemoryRequirements2KHR        = vkGetImageMemoryRequirements2;
            vmaVulkanFunctions.vkBindBufferMemory2KHR                  = vkBindBufferMemory2;
            vmaVulkanFunctions.vkBindImageMemory2KHR                   = vkBindImageMemory2;
            // Vulkan 1.3+
            vmaVulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
            vmaVulkanFunctions.vkGetDeviceImageMemoryRequirements  = vkGetDeviceImageMemoryRequirements;

            allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;

            if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create VMA allocator!");
            }

            return Resources::VulkanResource(
                instance,
                std::move(debugMessenger),
                surface,
                physicalDevice,
                device,
                additionalDeviceExtensions,
                allocator,
                graphicsQueue,
                presentationQueue,
                windowExtent);
        }
        catch (const std::exception& e) {
            std::cerr << "Couldn't load Vulkan - " << e.what() << std::endl;

            return std::nullopt;
        }
    }
}; // namespace Prism::Loaders