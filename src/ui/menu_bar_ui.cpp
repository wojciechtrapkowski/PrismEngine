#include <imgui.h>
#include <imgui_internal.h>
#include <nfd.h>

#include "ui/menu_bar_ui.hpp"

#include "events/file_events.hpp"

namespace Prism::UI
{
    MenuBarUI::MenuBarUI(Resources::ContextResources& contextResources) : _contextResources(contextResources)
    {
        NFD_Init();
    }

    MenuBarUI::~MenuBarUI()
    {
        NFD_Quit();
    }

    void MenuBarUI::Update(float deltaTime, Resources::Scene& scene)
    {
        auto& dispatcher = _contextResources.GetDispatcher();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                // if (ImGui::MenuItem("Open Scene")) {
                // }
                // if (ImGui::MenuItem("Save Scene")) {
                // }
                if (ImGui::MenuItem("Load Mesh")) {
                    nfdu8filteritem_t     filters[1] = {{"3D Models", "obj,fbx,gltf,glb,dae,3ds,ply,stl,blend,x,ms3d,lwo,md2,md3,md5mesh"}};
                    nfdopendialogu8args_t args       = {0};
                    args.filterList                  = filters;
                    args.filterCount                 = 1;

                    nfdu8char_t* outPath;
                    nfdresult_t  result = NFD_OpenDialogU8_With(&outPath, &args);
                    if (result == NFD_OKAY) {
                        dispatcher.enqueue<Events::MeshFileOpenEvent>({.filePath = outPath});

                        NFD_FreePathU8(outPath);
                    } else if (result == NFD_CANCEL) {
                        // Cancel.
                    } else {
                        // Error.
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
} // namespace Prism::UI