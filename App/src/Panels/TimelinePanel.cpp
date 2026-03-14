#include "Panels/TimelinePanel.h"
#include "Cyclops/UI/Theme.h"
#include "Cyclops/Tools/LayerCommands.h" 
#include "Cyclops/Core/CommandHistory.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <cstdio> 
#include <string>
#include <algorithm> 
#include <iostream> 

namespace Cyclops {

    void TimelinePanel::OnImGuiRender(Canvas* canvas, bool& isPlaying, int& fps, int& maxFrames)
    {
        ImGui::Begin("Timeline");

        // ============================================================
        // 1. TOP TOOLBAR (Fixed)
        // ============================================================

        if (isPlaying) {
            if (ImGui::Button("|| Stop")) isPlaying = false;
        }
        else {
            if (ImGui::Button("> Play")) isPlaying = true;
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::DragInt("FPS", &fps, 1.0f, 1, 60);
        ImGui::SameLine();
        ImGui::Text("Frame: %d / %d", canvas->GetCurrentFrame(), maxFrames);

        ImGui::SameLine();
        if (ImGui::Button("Clear All")) {
            isPlaying = false;
            canvas->Resize(800, 600);
            canvas->GetLayers().clear();
            canvas->AddLayer("Background");
            canvas->SetCurrentFrame(0);
        }

        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();

        // --- NEW KEYFRAME ---
        ImGui::PushStyleColor(ImGuiCol_Button, UI::Theme::AccentDim);
        if (ImGui::Button("+ Keyframe")) {
            int activeIdx = canvas->GetActiveLayerIndex();
            if (activeIdx >= 0 && activeIdx < canvas->GetLayers().size()) {
                canvas->GetLayers()[activeIdx].CreateKeyframe(canvas->GetCurrentFrame(), canvas->GetWidth(), canvas->GetHeight());
                canvas->Recompose();
            }
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // --- DUPLICATE KEYFRAME ---
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.1f, 1.0f));
        if (ImGui::Button("Duplicate")) {
            int activeIdx = canvas->GetActiveLayerIndex();
            if (activeIdx >= 0 && activeIdx < canvas->GetLayers().size()) {
                int currentFrame = canvas->GetCurrentFrame();
                auto& layer = canvas->GetLayers()[activeIdx];
                if (layer.Keyframes.find(currentFrame) == layer.Keyframes.end()) {
                    CommandHistory::AddCommand(std::make_shared<DuplicateKeyframeCommand>(
                        canvas, activeIdx, currentFrame
                    ));
                }
            }
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        bool onion = canvas->IsOnionSkinEnabled();
        if (ImGui::Checkbox("Onion Skin", &onion)) {
            canvas->SetOnionSkinEnabled(onion);
        }

        ImGui::Separator();

        // ============================================================
        // 2. SCROLLABLE TRACKS AREA
        // ============================================================

        float totalTimelineWidth = (maxFrames + 10) * m_FrameWidth;
        float totalTimelineHeight = (canvas->GetLayers().size() * m_RowHeight) + m_HeaderHeight + 50.0f;

        ImGui::SetNextWindowContentSize(ImVec2(totalTimelineWidth, totalTimelineHeight));
        ImGui::BeginChild("TracksArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        ImVec2 visibleSize = ImGui::GetWindowSize();
        float scrollX = ImGui::GetScrollX();

        // --- AUTO-SCROLL ---
        if (isPlaying)
        {
            float currentFrameX = canvas->GetCurrentFrame() * m_FrameWidth;
            if (currentFrameX < scrollX || currentFrameX > scrollX + visibleSize.x)
            {
                ImGui::SetScrollX(currentFrameX - (visibleSize.x * 0.5f));
            }
        }

        // --- HEADER ---
        ImRect headerRect(startPos, ImVec2(startPos.x + totalTimelineWidth, startPos.y + m_HeaderHeight));
        drawList->AddRectFilled(headerRect.Min, headerRect.Max, IM_COL32(40, 40, 40, 255));

        ImGui::SetCursorScreenPos(startPos);
        ImGui::InvisibleButton("##TimelineHeader", ImVec2(totalTimelineWidth, m_HeaderHeight));

        if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            float mouseX = ImGui::GetMousePos().x;
            float relativeX = mouseX - startPos.x;
            int targetFrame = (int)(relativeX / m_FrameWidth);
            if (targetFrame < 0) targetFrame = 0;
            if (targetFrame > maxFrames) targetFrame = maxFrames;
            canvas->SetCurrentFrame(targetFrame);
            isPlaying = false;
        }

        int firstVisibleFrame = (int)(scrollX / m_FrameWidth);
        int lastVisibleFrame = firstVisibleFrame + (int)(visibleSize.x / m_FrameWidth) + 2;
        if (lastVisibleFrame > maxFrames) lastVisibleFrame = maxFrames;

        // Draw Ticks
        for (int f = firstVisibleFrame; f <= lastVisibleFrame; f++)
        {
            float x = startPos.x + (f * m_FrameWidth);
            if (f % 5 == 0) {
                drawList->AddLine(ImVec2(x, startPos.y + 15), ImVec2(x, startPos.y + m_HeaderHeight), IM_COL32(100, 100, 100, 255));
                char numStr[16];
                sprintf_s(numStr, "%d", f);
                drawList->AddText(ImVec2(x + 2, startPos.y), IM_COL32(200, 200, 200, 255), numStr);
            }
            else {
                drawList->AddLine(ImVec2(x, startPos.y + 20), ImVec2(x, startPos.y + m_HeaderHeight), IM_COL32(60, 60, 60, 255));
            }
        }

        // --- LAYERS ---
        auto& layers = canvas->GetLayers();
        float startY = startPos.y + m_HeaderHeight;

        for (int i = (int)layers.size() - 1; i >= 0; i--)
        {
            auto& layer = layers[i];
            int rowIndex = (int)layers.size() - 1 - i;
            float y = startY + (rowIndex * m_RowHeight);

            if (i == canvas->GetActiveLayerIndex()) {
                drawList->AddRectFilled(
                    ImVec2(startPos.x, y),
                    ImVec2(startPos.x + totalTimelineWidth, y + m_RowHeight),
                    IM_COL32(60, 60, 80, 255)
                );
            }

            drawList->AddLine(
                ImVec2(startPos.x, y + m_RowHeight),
                ImVec2(startPos.x + totalTimelineWidth, y + m_RowHeight),
                IM_COL32(30, 30, 30, 255)
            );

            // --- MOVE KEYFRAME DROP TARGET ---
            ImGui::SetCursorScreenPos(ImVec2(startPos.x, y));
            ImGui::SetNextItemAllowOverlap();
            std::string rowDropID = "##RowDrop" + std::to_string(i);
            ImGui::InvisibleButton(rowDropID.c_str(), ImVec2(totalTimelineWidth, m_RowHeight));

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("KEYFRAME_MOVE")) {
                    int* data = (int*)payload->Data;
                    int srcLayer = data[0];
                    int srcFrame = data[1];
                    float dropX = ImGui::GetMousePos().x - startPos.x;
                    int targetFrame = (int)(dropX / m_FrameWidth);
                    if (targetFrame < 0) targetFrame = 0;

                    if (srcFrame != targetFrame || srcLayer != i) {
                        CommandHistory::AddCommand(std::make_shared<MoveKeyframeCommand>(canvas, srcLayer, srcFrame, targetFrame));
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // Keyframes
            for (auto it = layer.Keyframes.begin(); it != layer.Keyframes.end(); ++it)
            {
                int currentKeyFrame = it->first;
                if (currentKeyFrame < firstVisibleFrame - 1 || currentKeyFrame > lastVisibleFrame + 1) continue;

                float xStart = startPos.x + (currentKeyFrame * m_FrameWidth);
                float cy = y + (m_RowHeight * 0.5f);

                ImVec2 dotPos(xStart + (m_FrameWidth * 0.5f), cy);
                drawList->AddCircleFilled(dotPos, 5.0f, IM_COL32(255, 255, 255, 255));
                drawList->AddCircle(dotPos, 5.0f, IM_COL32(0, 0, 0, 255));

                float hitBoxSize = 16.0f;
                ImGui::SetCursorScreenPos(ImVec2(dotPos.x - (hitBoxSize * 0.5f), dotPos.y - (hitBoxSize * 0.5f)));
                std::string btnID = "##Key" + std::to_string(i) + "_" + std::to_string(currentKeyFrame);
                ImGui::InvisibleButton(btnID.c_str(), ImVec2(hitBoxSize, hitBoxSize));

                if (ImGui::BeginDragDropSource()) {
                    int payloadData[2] = { i, currentKeyFrame };
                    ImGui::SetDragDropPayload("KEYFRAME_MOVE", &payloadData, sizeof(payloadData));
                    ImGui::Text("Frame %d", currentKeyFrame);
                    ImGui::EndDragDropSource();
                }

                // --- DELETE KEYFRAME LOGIC (FIXED) ---
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Delete")) {
                        CommandHistory::AddCommand(std::make_shared<RemoveKeyframeCommand>(canvas, i, currentKeyFrame));

                        // [CRITICAL FIX] 
                        // Break out of the loop immediately because 'it' is now invalid.
                        // We will redraw correctly on the next frame.
                        ImGui::EndPopup();
                        break;
                    }
                    ImGui::EndPopup();
                }
            }
        }

        // --- PLAYHEAD ---
        float playheadX = startPos.x + (canvas->GetCurrentFrame() * m_FrameWidth);
        float playheadHeight = (layers.size() * m_RowHeight) + m_HeaderHeight;

        drawList->AddLine(ImVec2(playheadX, startPos.y), ImVec2(playheadX, startPos.y + playheadHeight), IM_COL32(255, 50, 50, 200), 1.5f);
        drawList->AddTriangleFilled(ImVec2(playheadX - 6, startPos.y), ImVec2(playheadX + 6, startPos.y), ImVec2(playheadX, startPos.y + 10), IM_COL32(255, 50, 50, 255));

        ImGui::EndChild();
        ImGui::End();
    }
}