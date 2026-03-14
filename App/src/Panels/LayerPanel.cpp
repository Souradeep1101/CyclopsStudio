#include "Panels/LayerPanel.h"

#include "Cyclops/Tools/LayerCommands.h"
#include "Cyclops/Core/CommandHistory.h"
#include "Cyclops/UI/Theme.h"

#include <imgui.h>
#include <string>
#include <cstring> 
#include <vector>

namespace Cyclops {

    // Helper to convert Enum to String for Dropdown
    static const char* BlendModeStrings[] = { "Normal", "Multiply", "Add (Glow)", "Screen", "Overlay" };

    void LayerPanel::OnImGuiRender(Canvas* canvas)
    {
        ImGui::Begin("Layers");

        // 1. Safety Check
        if (canvas->GetLayers().empty())
        {
            ImGui::Text("No Layers");
            ImGui::End();
            return;
        }

        int activeIdx = canvas->GetActiveLayerIndex();
        // Ensure index is valid
        if (activeIdx < 0 || activeIdx >= canvas->GetLayers().size()) activeIdx = 0;

        auto& activeLayer = canvas->GetLayers()[activeIdx];

        // ============================================================
        // A. THE CSP HEADER STRIP (Active Layer Controls)
        // ============================================================

        // --- Row 1: Blend Mode & Opacity ---
        ImGui::SetNextItemWidth(110);
        int currentBlend = (int)activeLayer.Blend;
        if (ImGui::Combo("##Blend", &currentBlend, BlendModeStrings, IM_ARRAYSIZE(BlendModeStrings)))
        {
            activeLayer.Blend = (BlendMode)currentBlend;
            canvas->Recompose();
        }

        ImGui::SameLine();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::SliderFloat("##Opacity", &activeLayer.Opacity, 0.0f, 1.0f, "Op: %.2f"))
        {
            canvas->Recompose();
        }

        // --- Row 2: Lock Toggles ---
        // Alpha Lock
        bool alphaLock = activeLayer.IsAlphaLocked;
        if (ImGui::Checkbox("Alpha Lock", &alphaLock))
        {
            activeLayer.IsAlphaLocked = alphaLock;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Lock Transparent Pixels");

        ImGui::SameLine();

        // Full Lock (Padlock)
        bool fullLock = activeLayer.IsLocked;
        if (ImGui::Checkbox("Lock", &fullLock))
        {
            activeLayer.IsLocked = fullLock;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Prevent all editing/moving");

        ImGui::Separator();

        // ============================================================
        // B. TOOLBAR (Add/Delete)
        // ============================================================

        ImGui::PushStyleColor(ImGuiCol_Button, UI::Theme::AccentDim);
        if (ImGui::Button("+ New"))
        {
            CommandHistory::AddCommand(std::make_shared<AddLayerCommand>(canvas));
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::BeginDisabled(!canvas->CanDeleteLayer());
        if (ImGui::Button("- Delete"))
        {
            CommandHistory::AddCommand(std::make_shared<RemoveLayerCommand>(canvas, activeIdx));
        }
        ImGui::EndDisabled();

        ImGui::Separator();

        // ============================================================
        // C. LAYER LIST (The Stack)
        // ============================================================

        ImGui::BeginChild("LayerList"); // Scrollable area

        // Iterate BACKWARDS (Top layer first in UI)
        auto& layers = canvas->GetLayers();
        for (int i = (int)layers.size() - 1; i >= 0; i--)
        {
            DrawLayerNode(canvas, i);
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void LayerPanel::DrawLayerNode(Canvas* canvas, int index)
    {
        auto& layer = canvas->GetLayers()[index];
        bool isActive = (canvas->GetActiveLayerIndex() == index);

        ImGui::PushID(index);

        // [VISUAL] Highlight Active Layer Row
        if (isActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, UI::Theme::Selection);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, UI::Theme::Selection);
        }

        // 1. Visibility Toggle (The "Eye")
        bool visible = layer.IsVisible;
        if (ImGui::Checkbox("##V", &visible))
        {
            layer.IsVisible = visible;
            canvas->Recompose();
        }
        ImGui::SameLine();

        // ============================================================
        // [NEW] 2. LIVE THUMBNAIL
        // ============================================================
        uint32_t texID = 0;
        auto fbo = layer.GetActiveFramebuffer(canvas->GetCurrentFrame());
        if (fbo)
        {
            texID = fbo->GetTextureID();
        }

        if (texID != 0)
        {
            // Draw Thumbnail (32x32) - UVs flipped (0,1) -> (1,0) for OpenGL
            ImGui::Image((void*)(intptr_t)texID, ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.5f)); // Grey border

            // Tooltip: Big Preview on Hover
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("%s", layer.Name.c_str());
                ImGui::Image((void*)(intptr_t)texID, ImVec2(200, 200), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::EndTooltip();
            }
        }
        else
        {
            // Placeholder for empty layers
            ImGui::Button("##Empty", ImVec2(32, 32));
        }
        ImGui::SameLine();
        // ============================================================

        // 3. Selectable Row (Name & Selection)
        // [FIX] Align text to center of thumbnail
        ImGui::AlignTextToFramePadding();

        // Allow Double Click to rename
        if (ImGui::Selectable(layer.Name.c_str(), isActive, ImGuiSelectableFlags_AllowDoubleClick))
        {
            canvas->SetActiveLayer(index);

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                ImGui::OpenPopup("RenameLayer");
            }
        }

        // 4. Status Icons (Right side of name)
        if (layer.IsLocked)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("(L)");
        }
        if (layer.IsAlphaLocked)
        {
            ImGui::SameLine();
            ImGui::TextDisabled("(A)");
        }

        if (isActive) ImGui::PopStyleColor(2);

        // 5. Rename Popup
        if (ImGui::BeginPopup("RenameLayer"))
        {
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, layer.Name.c_str(), sizeof(buffer));

            if (ImGui::IsWindowAppearing())
                ImGui::SetKeyboardFocusHere();

            if (ImGui::InputText("##Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                if (strlen(buffer) > 0)
                    layer.Name = std::string(buffer);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // 6. Drag and Drop (Reordering)
        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("LAYER_MOVE", &index, sizeof(int));
            ImGui::Text("Move %s", layer.Name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_MOVE"))
            {
                int fromIndex = *(const int*)payload->Data;
                if (fromIndex != index)
                {
                    CommandHistory::AddCommand(std::make_shared<MoveLayerCommand>(canvas, fromIndex, index));
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::PopID();
    }
}