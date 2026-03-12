#include "managers/context_update_systems_manager.hpp"

namespace Prism::Managers
{
    ContextUpdateSystemsManager::ContextUpdateSystemsManager(Resources::ContextResources& contextResources) :
        _contextResources{contextResources}, _eventPollSystem{contextResources}, _inputControlSystem{contextResources}, _windowResizeSystem{contextResources},
        _frameSwapSystem{contextResources}
    {}

    void ContextUpdateSystemsManager::Update(float deltaTime)
    {
        _eventPollSystem.Update(deltaTime);
        _inputControlSystem.Update(deltaTime);
        _windowResizeSystem.Update(deltaTime);
        _frameSwapSystem.Update(deltaTime);
    }
} // namespace Prism::Managers