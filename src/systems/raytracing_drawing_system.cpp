#include "systems/raytracing_drawing_system.hpp"

#include "components/mesh.hpp"
#include "components/transform.hpp"

#include "utils/vulkan/common.hpp"

#include "resources/common_resource.hpp"
#include "resources/vulkan/vk_buffer_resource.hpp"
#include "resources/vulkan/vk_acceleration_structure_resource.hpp"

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <iostream>
#include <vector>

#include "vulkan/vulkan.h"

namespace Prism::Systems
{
    namespace
    {
        constexpr auto MIN_ACCELERATION_STRUCTURE_SCRATCH_OFFSET_ALIGNMENT = 128;

        constexpr auto getMeshBLASId = [](Resources::MeshResource::ID id) {
            return std::hash<std::string_view>{}(std::format("RaytracingDrawingSystem/BLAS/{}", id));
        };

        VkDescriptorPool createDescriptorPool(VkDevice device)
        {
            VkDescriptorPool descriptorPool;

            std::array<VkDescriptorPoolSize, 1> poolSizes{};
            poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = Resources::VulkanResource::FRAMES_IN_FLIGHT;

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.maxSets       = Resources::VulkanResource::FRAMES_IN_FLIGHT;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes    = poolSizes.data();

            if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create descriptor pool!");
            }

            return descriptorPool;
        }

        VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device)
        {
            VkDescriptorSetLayout descriptorSetLayout;

            VkDescriptorSetLayoutBinding uboBinding{};
            uboBinding.binding            = 0;
            uboBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboBinding.descriptorCount    = 1;
            uboBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            uboBinding.pImmutableSamplers = nullptr;

            std::array<VkDescriptorSetLayoutBinding, 1> bindings = {uboBinding};

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings    = bindings.data();

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout");
            }

            return descriptorSetLayout;
        }

        std::vector<VkDescriptorSet> createDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout)
        {
            std::vector<VkDescriptorSet> descriptorSets;

            descriptorSets.resize(Resources::VulkanResource::FRAMES_IN_FLIGHT);

            std::vector<VkDescriptorSetLayout> layouts(Resources::VulkanResource::FRAMES_IN_FLIGHT, descriptorSetLayout);

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool     = descriptorPool;
            allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
            allocInfo.pSetLayouts        = layouts.data();

            if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate descriptor sets!");
            }

            return descriptorSets;
        }

        VkPipelineLayout createPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout)
        {
            VkPipelineLayout pipelineLayout;

            VkPushConstantRange pushRange{};
            pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            pushRange.offset     = 0;
            pushRange.size       = sizeof(glm::mat4);

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount         = 1;
            pipelineLayoutInfo.pSetLayouts            = &descriptorSetLayout;
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges    = &pushRange;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create pipeline layout!");
            }

            return pipelineLayout;
        }

        VkPipeline createPipeline(VkDevice device, VkPipelineLayout pipelineLayout)
        {
            // Load shader modules
            VkShaderModule vertexShaderModule   = Utils::Vulkan::Common::loadShaderModule(device, BASIC_VERT_SHADER_PATH);
            VkShaderModule fragmentShaderModule = Utils::Vulkan::Common::loadShaderModule(device, BASIC_FRAG_SHADER_PATH);

            // Shader stages
            VkPipelineShaderStageCreateInfo shaderStages[2]{};

            shaderStages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].module = vertexShaderModule;
            shaderStages[0].pName  = "main";

            shaderStages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].module = fragmentShaderModule;
            shaderStages[1].pName  = "main";

            // Vertex input state
            VkPipelineVertexInputStateCreateInfo vertexInputState{};
            vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            VkVertexInputBindingDescription binding{};
            binding.binding   = 0;
            binding.stride    = sizeof(Resources::MeshResource::Vertex);
            binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            VkVertexInputAttributeDescription attributes[3]{};
            attributes[0].binding  = 0;
            attributes[0].location = 0;
            attributes[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attributes[0].offset   = offsetof(Resources::MeshResource::Vertex, position);

            attributes[1].binding  = 0;
            attributes[1].location = 1;
            attributes[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
            attributes[1].offset   = offsetof(Resources::MeshResource::Vertex, normal);

            attributes[2].binding  = 0;
            attributes[2].location = 2;
            attributes[2].format   = VK_FORMAT_R32G32_SFLOAT;
            attributes[2].offset   = offsetof(Resources::MeshResource::Vertex, textureUV);

            vertexInputState.vertexBindingDescriptionCount   = 1;
            vertexInputState.pVertexBindingDescriptions      = &binding;
            vertexInputState.vertexAttributeDescriptionCount = 3;
            vertexInputState.pVertexAttributeDescriptions    = attributes;

            // Input assembly
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
            inputAssemblyState.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            // Viewport and scissor state
            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.scissorCount  = 1;

            // Rasterizer
            VkPipelineRasterizationStateCreateInfo rasterizationState{};
            rasterizationState.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizationState.cullMode    = VK_CULL_MODE_NONE;
            rasterizationState.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizationState.lineWidth   = 1.0f;

            // Multisampling
            VkPipelineMultisampleStateCreateInfo multisampleState{};
            multisampleState.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            // Depth and stencil state
            VkPipelineDepthStencilStateCreateInfo depthStencilState{};
            depthStencilState.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilState.depthTestEnable  = VK_TRUE;
            depthStencilState.depthWriteEnable = VK_TRUE;
            depthStencilState.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;

            // Color blend attachment
            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable    = VK_FALSE;

            // Color blend state
            VkPipelineColorBlendStateCreateInfo colorBlendState{};
            colorBlendState.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendState.attachmentCount = 1;
            colorBlendState.pAttachments    = &colorBlendAttachment;

            // Dynamic states
            VkDynamicState                   dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
            dynamicStateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(std::size(dynamicStates));
            dynamicStateInfo.pDynamicStates    = dynamicStates;

            // Dynamic rendering info (no render pass)
            VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;

            VkPipelineRenderingCreateInfo renderingInfo{};
            renderingInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
            renderingInfo.colorAttachmentCount    = 1;
            renderingInfo.pColorAttachmentFormats = &colorFormat;
            renderingInfo.depthAttachmentFormat   = VK_FORMAT_D32_SFLOAT_S8_UINT;
            renderingInfo.stencilAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

            // Graphics pipeline create info
            VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
            pipelineCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.pNext               = &renderingInfo;
            pipelineCreateInfo.stageCount          = 2;
            pipelineCreateInfo.pStages             = shaderStages;
            pipelineCreateInfo.pVertexInputState   = &vertexInputState;
            pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
            pipelineCreateInfo.pViewportState      = &viewportState;
            pipelineCreateInfo.pRasterizationState = &rasterizationState;
            pipelineCreateInfo.pMultisampleState   = &multisampleState;
            pipelineCreateInfo.pDepthStencilState  = &depthStencilState;
            pipelineCreateInfo.pColorBlendState    = &colorBlendState;
            pipelineCreateInfo.pDynamicState       = &dynamicStateInfo;
            pipelineCreateInfo.layout              = pipelineLayout;
            pipelineCreateInfo.renderPass          = VK_NULL_HANDLE; // dynamic rendering
            pipelineCreateInfo.basePipelineHandle  = VK_NULL_HANDLE;

            // Create graphics pipeline
            VkPipeline pipeline{};
            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
                vkDestroyShaderModule(device, vertexShaderModule, nullptr);
                vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
                throw std::runtime_error("vkCreateGraphicsPipelines failed");
            }

            // Cleanup shader modules
            vkDestroyShaderModule(device, vertexShaderModule, nullptr);
            vkDestroyShaderModule(device, fragmentShaderModule, nullptr);

            return pipeline;
        }

        void updateDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet, VkBuffer commonUniformBuffer)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = commonUniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range  = VK_WHOLE_SIZE;

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = descriptorSet;
            descriptorWrite.dstBinding      = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo     = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        };

        Resources::VkAccelerationStructureResource
        createAccelerationStructureFromMesh(Resources::VulkanResource& vulkan, VkCommandBuffer commandBuffer, Resources::MeshResource& mesh)
        {
            auto pfnGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
                vkGetDeviceProcAddr(vulkan.GetDevice(), "vkGetAccelerationStructureBuildSizesKHR"));
            auto pfnCreateAccelerationStructureKHR =
                reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(vulkan.GetDevice(), "vkCreateAccelerationStructureKHR"));
            auto pfnCmdBuildAccelerationStructuresKHR =
                reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(vulkan.GetDevice(), "vkCmdBuildAccelerationStructuresKHR"));

            VkAccelerationStructureKHR accelStruct{};

            // Prepare geometry

            const auto triangleCount = static_cast<uint32_t>(mesh.GetIndexBuffer().GetElementCount() / 3U);

            auto& vertexBuffer = mesh.GetVertexBuffer();
            auto& indexBuffer  = mesh.GetIndexBuffer();

            VkAccelerationStructureGeometryTrianglesDataKHR accelTriangles{
                .sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                .vertexFormat = mesh.GetVertexType(),
                .vertexData   = {.deviceAddress = vertexBuffer.GetBufferDeviceAddress()},
                .vertexStride = vertexBuffer.GetElementSize(),
                .maxVertex    = static_cast<uint32_t>(vertexBuffer.GetElementCount()),
                .indexType    = mesh.GetIndexType(),
                .indexData    = {.deviceAddress = indexBuffer.GetBufferDeviceAddress()},
            };

            VkAccelerationStructureGeometryKHR accelGeometry = VkAccelerationStructureGeometryKHR{
                .sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
                .geometry     = {.triangles = accelTriangles},
                .flags        = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR | VK_GEOMETRY_OPAQUE_BIT_KHR,
            };

            VkAccelerationStructureBuildRangeInfoKHR accelRangeInfo{.primitiveCount = triangleCount};

            VkAccelerationStructureBuildGeometryInfoKHR accelBuildInfo{
                .sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                .type          = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
                .flags         = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
                .mode          = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                .geometryCount = 1,
                .pGeometries   = &accelGeometry,
            };

            std::vector<uint32_t> maxPrimCount(1);
            maxPrimCount[0] = accelRangeInfo.primitiveCount;

            // Create buffers for acceleration structure and scratch space
            VkAccelerationStructureBuildSizesInfoKHR accelBuildSize{.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
            pfnGetAccelerationStructureBuildSizesKHR(
                vulkan.GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &accelBuildInfo, maxPrimCount.data(), &accelBuildSize);

            Resources::VkBufferResource<> accelerationStructureBuffer{
                vulkan.GetVmaAllocator(),
                accelBuildSize.accelerationStructureSize,
                VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT};

            Resources::VkBufferResource<> scratchBuffer{
                vulkan.GetVmaAllocator(),
                accelBuildSize.buildScratchSize,
                VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT,
                MIN_ACCELERATION_STRUCTURE_SCRATCH_OFFSET_ALIGNMENT};

            VkAccelerationStructureCreateInfoKHR accelCreateInfo{
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
                .size  = accelBuildSize.accelerationStructureSize,
                .type  = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            };

            accelCreateInfo.buffer = accelerationStructureBuffer.GetBuffer();

            // Create and build acceleration structure

            if (pfnCreateAccelerationStructureKHR(vulkan.GetDevice(), &accelCreateInfo, nullptr, &accelStruct) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create acceleration structure!");
            }

            accelBuildInfo.dstAccelerationStructure  = accelStruct;
            accelBuildInfo.scratchData.deviceAddress = scratchBuffer.GetBufferDeviceAddress();

            VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &accelRangeInfo;

            pfnCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &accelBuildInfo, &pBuildRangeInfo);

            return {vulkan.GetDevice(), std::move(accelerationStructureBuffer), std::move(scratchBuffer), std::move(accelStruct)};
        }

        // VkTransformMatrixKHR is row-major 3x4, glm::mat4 is column-major; transpose before memcpy
        auto toTransformMatrixKHR = [](const glm::mat4& m) {
            VkTransformMatrixKHR t;
            memcpy(&t, glm::value_ptr(glm::transpose(m)), sizeof(t));
            return t;
        };

        Resources::VkAccelerationStructureResource createTLAS(
            Resources::VulkanResource&                       vulkan,
            VkCommandBuffer                                  commandBuffer,
            Resources::VkBufferResource<>&                   tlasInstancesBuffer,
            std::vector<VkAccelerationStructureInstanceKHR>& instances)
        {
            auto pfnGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
                vkGetDeviceProcAddr(vulkan.GetDevice(), "vkGetAccelerationStructureBuildSizesKHR"));
            auto pfnCreateAccelerationStructureKHR =
                reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(vulkan.GetDevice(), "vkCreateAccelerationStructureKHR"));
            auto pfnCmdBuildAccelerationStructuresKHR =
                reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(vulkan.GetDevice(), "vkCmdBuildAccelerationStructuresKHR"));

            VkAccelerationStructureGeometryInstancesDataKHR geometryInstances{
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                .data  = {.deviceAddress = tlasInstancesBuffer.GetBufferDeviceAddress()}};

            VkAccelerationStructureGeometryKHR accelGeometry = {
                .sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
                .geometry     = {.instances = geometryInstances}};

            VkAccelerationStructureBuildRangeInfoKHR accelBuildRangeInfo = {.primitiveCount = static_cast<uint32_t>(instances.size())};

            constexpr auto MIN_ACCELERATION_STRUCTURE_SCRATCH_OFFSET_ALIGNMENT = 128;

            VkAccelerationStructureKHR accelStruct{};

            // Prepare geometry

            VkAccelerationStructureBuildGeometryInfoKHR accelBuildInfo{
                .sType         = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                .type          = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
                .flags         = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
                .mode          = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                .geometryCount = 1,
                .pGeometries   = &accelGeometry,
            };

            std::vector<uint32_t> maxPrimCount(1);
            maxPrimCount[0] = accelBuildRangeInfo.primitiveCount;

            // Create buffers for acceleration structure and scratch space
            VkAccelerationStructureBuildSizesInfoKHR accelBuildSize{.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
            pfnGetAccelerationStructureBuildSizesKHR(
                vulkan.GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &accelBuildInfo, maxPrimCount.data(), &accelBuildSize);

            Resources::VkBufferResource<> accelerationStructureBuffer{
                vulkan.GetVmaAllocator(),
                accelBuildSize.accelerationStructureSize,
                VK_BUFFER_USAGE_2_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT};

            Resources::VkBufferResource<> scratchBuffer{
                vulkan.GetVmaAllocator(),
                accelBuildSize.buildScratchSize,
                VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT,
                MIN_ACCELERATION_STRUCTURE_SCRATCH_OFFSET_ALIGNMENT};

            VkAccelerationStructureCreateInfoKHR accelCreateInfo{
                .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
                .size  = accelBuildSize.accelerationStructureSize,
                .type  = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
            };

            accelCreateInfo.buffer = accelerationStructureBuffer.GetBuffer();

            // Create and build acceleration structure

            if (pfnCreateAccelerationStructureKHR(vulkan.GetDevice(), &accelCreateInfo, nullptr, &accelStruct) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create acceleration structure!");
            }

            accelBuildInfo.dstAccelerationStructure  = accelStruct;
            accelBuildInfo.scratchData.deviceAddress = scratchBuffer.GetBufferDeviceAddress();

            VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &accelBuildRangeInfo;

            pfnCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &accelBuildInfo, &pBuildRangeInfo);

            return {vulkan.GetDevice(), std::move(accelerationStructureBuffer), std::move(scratchBuffer), std::move(accelStruct)};
        }
    } // namespace

    RaytracingDrawingSystem::RaytracingDrawingSystem(Resources::ContextResources& contextResources) : _contextResources(contextResources)
    {
        auto&    vulkanResource = _contextResources.GetVulkanResource();
        VkDevice device         = vulkanResource.GetDevice();

        _descriptorPool      = createDescriptorPool(device);
        _descriptorSetLayout = createDescriptorSetLayout(device);
        _descriptorSets      = createDescriptorSets(device, _descriptorPool, _descriptorSetLayout);
        _pipelineLayout      = createPipelineLayout(device, _descriptorSetLayout);
        _pipeline            = createPipeline(device, _pipelineLayout);
    };

    RaytracingDrawingSystem::~RaytracingDrawingSystem()
    {
        auto&    vulkanResource = _contextResources.GetVulkanResource();
        VkDevice device         = vulkanResource.GetDevice();

        if (_pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, _pipeline, nullptr);
            _pipeline = VK_NULL_HANDLE;
        }
        if (_pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
            _pipelineLayout = VK_NULL_HANDLE;
        }
        if (_descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);
            _descriptorSetLayout = VK_NULL_HANDLE;
        }
        if (_descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
            _descriptorPool = VK_NULL_HANDLE;
        }
    }

    void RaytracingDrawingSystem::Initialize(){

    };

    void
    RaytracingDrawingSystem::Update(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::VkStagingBufferResource& stagingBuffer)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        auto& resourceStorage = _contextResources.GetResourceStorage();

        auto meshView = scene.GetRegistry().view<Components::Mesh, Components::Transform>();

        std::vector<Resources::MeshResource::ID> meshWithoutBLAS;
        meshWithoutBLAS.reserve(meshView.size_hint());

        bool updateOfInstanceDataBufferIsNeeded = false;

        for (auto& meshEntity : meshView) {
            auto& meshComponent      = meshView.get<Components::Mesh>(meshEntity);
            auto& transformComponent = meshView.get<Components::Transform>(meshEntity);
            auto& meshResourceId     = meshComponent.resourceId;
            auto  blasMeshResourceId = getMeshBLASId(meshResourceId);

            auto meshOpt = scene.GetMesh(meshResourceId);
            if (!meshOpt) {
                continue;
            }
            auto& mesh = meshOpt.value();

            auto accelStructureResourceOpt = resourceStorage.Get<Resources::VkAccelerationStructureResource>(blasMeshResourceId);
            if (!accelStructureResourceOpt) {
                auto accel = createAccelerationStructureFromMesh(_contextResources.GetVulkanResource(), commandBuffer, mesh);

                resourceStorage.Insert<Resources::VkAccelerationStructureResource>(
                    blasMeshResourceId, std::make_unique<Resources::VkAccelerationStructureResource>(std::move(accel)));
            }

            if (_blasToInstanceData.find(blasMeshResourceId) == _blasToInstanceData.end()) {
                _blasToInstanceData[blasMeshResourceId].push_back(transformComponent);
                updateOfInstanceDataBufferIsNeeded |= true;
            }
        }

        if (updateOfInstanceDataBufferIsNeeded) {
            std::vector<VkAccelerationStructureInstanceKHR> tlasInstances;

            // Optimization.
            size_t tlasInstancesVectorSize = 0;
            for (const auto& [_, instances] : _blasToInstanceData) {
                tlasInstancesVectorSize += instances.size();
            }
            tlasInstances.reserve(tlasInstancesVectorSize);

            for (const auto& [blasMeshResourceId, instances] : _blasToInstanceData) {
                auto blasOpt = resourceStorage.Get<Resources::VkAccelerationStructureResource>(blasMeshResourceId);
                if (!blasOpt) {
                    continue;
                }
                auto& blas          = blasOpt->get();
                auto  instanceIndex = 0;
                for (const auto& instance : instances) {
                    VkAccelerationStructureInstanceKHR asInstance{};
                    asInstance.transform                              = toTransformMatrixKHR(instance.transform);
                    asInstance.instanceCustomIndex                    = instanceIndex;
                    asInstance.accelerationStructureReference         = blas.GetAccelerationStructureBuffer().GetBufferDeviceAddress();
                    asInstance.instanceShaderBindingTableRecordOffset = 0; // We will use the same hit group for all objects
                    asInstance.flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV; // No culling - double sided
                    asInstance.mask                                   = 0xFF;
                    tlasInstances.emplace_back(asInstance);

                    instanceIndex++;
                }
            }

            resourceStorage.Delete(TLAS_INSTANCES_BUFFER_ID);

            auto& vmaAllocator = _contextResources.GetVulkanResource().GetVmaAllocator();

            Resources::VkBufferResource<> tlasInstancesBuffer{
                vmaAllocator,
                sizeof(VkAccelerationStructureInstanceKHR) * tlasInstances.size(),
                VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT |
                    VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU};

            stagingBuffer.Copy(tlasInstancesBuffer.GetBuffer(), tlasInstances.data(), sizeof(VkAccelerationStructureInstanceKHR) * tlasInstances.size());

            auto tlasAccelStruct = createTLAS(_contextResources.GetVulkanResource(), commandBuffer, tlasInstancesBuffer, tlasInstances);

            resourceStorage.Insert<Resources::VkAccelerationStructureResource>(
                TLAS_ACCEL_STRUCT_BUFFER_ID, std::make_unique<Resources::VkAccelerationStructureResource>(std::move(tlasAccelStruct)));

            resourceStorage.Insert<Resources::VkBufferResource<>>(
                TLAS_INSTANCES_BUFFER_ID, std::make_unique<Resources::VkBufferResource<>>(std::move(tlasInstancesBuffer)));
        }

        for (auto& entity : meshView) {
            auto& meshComponent      = meshView.get<Components::Mesh>(entity);
            auto& meshResourceId     = meshComponent.resourceId;
            auto  blasMeshResourceId = getMeshBLASId(meshResourceId);

            auto accelStructureResourceOpt = resourceStorage.Get<Resources::VkAccelerationStructureResource>(blasMeshResourceId);
            // It must have value now.
            auto& accelStructResource = accelStructureResourceOpt->get();

            accelStructResource.DestroyScratchBuffer();
        }

        vkEndCommandBuffer(commandBuffer);
    };

    void RaytracingDrawingSystem::Render(float deltaTime, VkCommandBuffer commandBuffer, Resources::Scene& scene, Resources::RenderTargetResource& renderTarget)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        auto& registry = scene.GetRegistry();

        auto& resourceStorage = _contextResources.GetResourceStorage();
        auto& vulkanResource  = _contextResources.GetVulkanResource();

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView   = renderTarget.GetColorImageView();
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingAttachmentInfo depthAttachment{};
        depthAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView   = renderTarget.GetDepthImageView();
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;

        auto currentSwapchainExtent = vulkanResource.GetSwapchainExtent();

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset    = {0, 0};
        renderingInfo.renderArea.extent    = {currentSwapchainExtent.width, currentSwapchainExtent.height};
        renderingInfo.layerCount           = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments    = &colorAttachment;
        renderingInfo.pDepthAttachment     = &depthAttachment;

        auto currentFrame = vulkanResource.GetCurrentFrameOffset();

        auto commonUniformBufferOpt =
            resourceStorage.Get<Resources::VkBufferResource<Resources::CommonResource>>(Resources::CommonResource::UNIFORM_BUFFER_ID, currentFrame);
        if (!commonUniformBufferOpt) {
            vkEndCommandBuffer(commandBuffer);
            return;
        }
        auto& commonUniformBuffer = commonUniformBufferOpt->get();

        updateDescriptorSet(vulkanResource.GetDevice(), _descriptorSets[currentFrame], commonUniformBuffer.GetBuffer());

        vkCmdBeginRendering(commandBuffer, &renderingInfo);

        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<float>(currentSwapchainExtent.width);
        viewport.height   = static_cast<float>(currentSwapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {currentSwapchainExtent.width, currentSwapchainExtent.height};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[currentFrame], 0, nullptr);

        vkCmdEndRendering(commandBuffer);

        vkEndCommandBuffer(commandBuffer);
    }
} // namespace Prism::Systems