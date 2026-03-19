#include "managers/scene_update_systems_manager.hpp"

namespace Prism::Managers
{
    SceneUpdateSystemsManager::SceneUpdateSystemsManager(Resources::ContextResources& contextResources) :
        cameraCreationSystem{contextResources}, motionControlSystem{contextResources}, commonUniformUpdateSystem{contextResources}
    {}

    void SceneUpdateSystemsManager::Update(float deltaTime, Resources::Scene& scene)
    {
        cameraCreationSystem.Update(deltaTime, scene);
        motionControlSystem.Update(deltaTime, scene);
        commonUniformUpdateSystem.Update(deltaTime, scene);
    }
} // namespace Prism::Managers