#include "managers/scene_draw_systems_manager.hpp"

namespace Prism::Managers
{
    namespace
    {
        std::vector<VkSemaphore> createSemaphores(VkDevice device, size_t count)
        {
            std::vector<VkSemaphore> semaphores(count);

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            for (auto& sem : semaphores) {
                if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &sem) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create semaphore!");
                }
            }

            return semaphores;
        }

        std::vector<Resources::VkCommandPoolResource> createCommandPools(VkDevice device, uint32_t graphicsQueueFamilyIndex, size_t count)
        {
            std::vector<Resources::VkCommandPoolResource> pools;
            pools.reserve(count);

            for (size_t i = 0; i < count; ++i) {
                pools.emplace_back(device, graphicsQueueFamilyIndex);
            }

            return pools;
        }

    } // namespace

    SceneDrawSystemsManager::SceneDrawSystemsManager(Resources::ContextResources& contextResources) :
        _contextResources(contextResources), _screenClearingSystem{contextResources}, _meshDrawingSystem{contextResources},
        _raytracingDrawingSystem{contextResources}, _uiDrawingSystem{contextResources}, _presentSystem{contextResources}, _gizmoDrawingSystem{contextResources}
    {
        auto& vulkanResource           = _contextResources.GetVulkanResource();
        auto  device                   = vulkanResource.GetDevice();
        auto  graphicsQueueFamilyIndex = vulkanResource.GetGraphicsQueueFamilyIndex();
        auto  framesInFlight           = vulkanResource.GetFramesInFlight();

        _commandPools = createCommandPools(device, graphicsQueueFamilyIndex, framesInFlight);

        _updateSemaphores = createSemaphores(device, framesInFlight);
        _renderSemaphores = createSemaphores(device, framesInFlight);
    }

    SceneDrawSystemsManager::~SceneDrawSystemsManager()
    {
        auto& vulkanResource = _contextResources.GetVulkanResource();

        for (auto& sem : _updateSemaphores) {
            vkDestroySemaphore(vulkanResource.GetDevice(), sem, nullptr);
        }
        for (auto& sem : _renderSemaphores) {
            vkDestroySemaphore(vulkanResource.GetDevice(), sem, nullptr);
        }
    }

    void SceneDrawSystemsManager::Initialize()
    {
        _screenClearingSystem.Initialize();
        _meshDrawingSystem.Initialize();
        _raytracingDrawingSystem.Initialize();
        _gizmoDrawingSystem.Initialize();
        _uiDrawingSystem.Initialize();
        _presentSystem.Initialize();
    }

    void SceneDrawSystemsManager::Update(float deltaTime, Resources::Scene& scene, Resources::VkStagingBufferResource& stagingBuffer)
    {
        auto& vulkanResource                = _contextResources.GetVulkanResource();
        auto& swapchainBoundResourceStorage = vulkanResource.GetSwapchainBoundStorage();

        auto& currentCommandPoolResource = _commandPools.at(vulkanResource.GetCurrentFrameOffset());
        auto& currentUpdateSemaphore     = _updateSemaphores.at(vulkanResource.GetCurrentFrameOffset());
        auto& currentRenderSemaphore     = _renderSemaphores.at(vulkanResource.GetCurrentFrameOffset());
        auto  imageAcquiredSemaphore     = vulkanResource.GetCurrentImageAcquiredSemaphore();
        auto  currentFence               = vulkanResource.GetCurrentFence();

        // This could be probably moved to frame swap system.
        vulkanResource.AdvanceFrame();

        currentCommandPoolResource.Reset();

        auto renderTargetOpt =
            swapchainBoundResourceStorage.Get<Resources::RenderTargetResource>(RENDER_TARGET_RESOURCE_ID, vulkanResource.GetCurrentImageIndex());
        if (!renderTargetOpt) {
            uint32_t flags = 0;
            flags |= Resources::RenderTargetResource::RenderTargetCreationFlags::COLOR_ATTACHMENT;
            flags |= Resources::RenderTargetResource::RenderTargetCreationFlags::DEPTH_STENCIL_ATTACHMENT;

            auto renderTarget = std::make_unique<Resources::RenderTargetResource>(
                vulkanResource.GetDevice(), vulkanResource.GetVmaAllocator(), vulkanResource.GetSwapchainExtent(), flags);

            swapchainBoundResourceStorage.Insert<Resources::RenderTargetResource>(
                RENDER_TARGET_RESOURCE_ID, std::move(renderTarget), vulkanResource.GetCurrentImageIndex());
            renderTargetOpt =
                swapchainBoundResourceStorage.Get<Resources::RenderTargetResource>(RENDER_TARGET_RESOURCE_ID, vulkanResource.GetCurrentImageIndex());
        }
        auto& renderTarget = renderTargetOpt->get();

        { // Update
            auto commandBuffersScope = currentCommandPoolResource.BeginScope();

            stagingBuffer.Commit(commandBuffersScope.GetNextCommandBuffer());

            _screenClearingSystem.Update(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene);
            _meshDrawingSystem.Update(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene);
            _raytracingDrawingSystem.Update(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene, stagingBuffer);
            _uiDrawingSystem.Update(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene);
            _gizmoDrawingSystem.Update(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene);
            _presentSystem.Update(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene);

            VkSubmitInfo submitInfo{};
            submitInfo.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount     = 0;
            submitInfo.pWaitSemaphores        = nullptr;
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
            submitInfo.pWaitDstStageMask      = waitStages;
            submitInfo.commandBufferCount     = static_cast<uint32_t>(commandBuffersScope.size());
            submitInfo.pCommandBuffers        = commandBuffersScope.data();
            submitInfo.signalSemaphoreCount   = 1;
            submitInfo.pSignalSemaphores      = &currentUpdateSemaphore;

            vkQueueSubmit(vulkanResource.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        }

        { // Render
            auto commandBuffersScope = currentCommandPoolResource.BeginScope();

            _screenClearingSystem.Render(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene, renderTarget);
            //_meshDrawingSystem.Render(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene, renderTarget);
            _raytracingDrawingSystem.Render(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene, renderTarget);
            _uiDrawingSystem.Render(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene, renderTarget);
            _gizmoDrawingSystem.Render(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene, renderTarget);
            _presentSystem.Render(deltaTime, commandBuffersScope.GetNextCommandBuffer(), scene, renderTarget);

            VkSubmitInfo submitInfo{};
            submitInfo.sType                      = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            VkSemaphore          waitSemaphores[] = {currentUpdateSemaphore, imageAcquiredSemaphore};
            VkPipelineStageFlags waitStages[]     = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount         = 2;
            submitInfo.pWaitSemaphores            = waitSemaphores;
            submitInfo.pWaitDstStageMask          = waitStages;
            submitInfo.commandBufferCount         = static_cast<uint32_t>(commandBuffersScope.size());
            submitInfo.pCommandBuffers            = commandBuffersScope.data();
            submitInfo.signalSemaphoreCount       = 1;
            VkSemaphore signalSemaphores[]        = {currentRenderSemaphore};
            submitInfo.pSignalSemaphores          = signalSemaphores;

            vkQueueSubmit(vulkanResource.GetGraphicsQueue(), 1, &submitInfo, currentFence);
        }

        { // Present
            auto currentImageIndex = vulkanResource.GetCurrentImageIndex();

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores    = &currentRenderSemaphore;
            VkSwapchainKHR swapchains[]    = {vulkanResource.GetSwapchain()};
            presentInfo.swapchainCount     = 1;
            presentInfo.pSwapchains        = swapchains;
            presentInfo.pImageIndices      = &currentImageIndex;

            vkQueuePresentKHR(vulkanResource.GetPresentationQueue(), &presentInfo);
        }
    }
} // namespace Prism::Managers