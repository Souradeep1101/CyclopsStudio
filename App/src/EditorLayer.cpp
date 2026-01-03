#include "EditorLayer.h"
#include "Engine.h"
#include "OpenGLDebug.h"
#include "Debug/Instrumentor.h"

#include <iostream>
#include <glad/glad.h>
#include <Events/KeyEvent.h>

namespace Cyclops
{
	void EditorLayer::OnAttach()
	{
        // 1. Create the Project Document (Canvas)
        // The Canvas constructor handles creating the Framebuffer 
        // and clearing it to white automatically.
        m_Canvas = std::make_unique<Canvas>(m_CanvasWidth, m_CanvasHeight);

        // 2. Init Brush Engine
        m_BrushEngine = std::make_unique<BrushEngine>();
        m_BrushEngine->Init();
	}

    void EditorLayer::OnDetach()
    {
        std::cout << "EditorLayer Detached! Resources freed." << std::endl;
    }

	void EditorLayer::DrawViewport()
	{
        CYCLOPS_PROFILE_FUNCTION();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");

        // --- 1. Input Handling (Pan & Zoom) ---
        ImGuiIO& io = ImGui::GetIO();

        if (ImGui::IsWindowHovered())
        {
            // Zoom
            if (io.MouseWheel != 0.0f)
            {
                m_ZoomLevel += io.MouseWheel * 0.1f;
                if (m_ZoomLevel < 0.1f) m_ZoomLevel = 0.1f;
                if (m_ZoomLevel > 5.0f) m_ZoomLevel = 5.0f;
            }
            // Pan
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
            {
                m_PanOffset.x += io.MouseDelta.x;
                m_PanOffset.y += io.MouseDelta.y;
            }
        }

        // --- 2. Calculate Geometry ---
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        ImVec2 center = { viewportSize.x * 0.5f, viewportSize.y * 0.5f };
        ImVec2 imageDisplaySize = { m_CanvasWidth * m_ZoomLevel, m_CanvasHeight * m_ZoomLevel };

        // Top-left corner of the image in "ImGui Window Space"
        ImVec2 imageTopLeft = {
            center.x - (imageDisplaySize.x * 0.5f) + m_PanOffset.x,
            center.y - (imageDisplaySize.y * 0.5f) + m_PanOffset.y
        };

        // --- 3. Draw The Texture ---
        ImGui::SetCursorPos(imageTopLeft);
        ImGui::Image((void*)(intptr_t)m_Canvas->GetTextureID(),
            imageDisplaySize,
            ImVec2(0, 1), ImVec2(1, 0));

        // --- 4. Coordinate System Logic (Crucial for Phase 4) ---
        if (ImGui::IsItemHovered())
        {
            ImVec2 mousePosScreen = ImGui::GetMousePos();
            ImVec2 windowPos = ImGui::GetWindowPos();

            // Mouse relative to the image top-left
            float mouseRelX = mousePosScreen.x - (windowPos.x + imageTopLeft.x);
            float mouseRelY = mousePosScreen.y - (windowPos.y + imageTopLeft.y);

            // Convert to Canvas Pixel Coordinates
            int pixelX = (int)(mouseRelX / m_ZoomLevel);
            int pixelY = (int)(mouseRelY / m_ZoomLevel);

            // Logic for Phase 4:
            // If (ImGui::IsMouseDown(0)) { BrushEngine::Paint(pixelX, pixelY); }
            // Inside the Input Logic block:

            if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                m_BrushEngine->BeginStroke(m_Canvas->GetActiveFramebuffer(), pixelX, pixelY);
            }

            if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                m_BrushEngine->ContinueStroke(pixelX, pixelY);
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                m_BrushEngine->EndStroke();
            }

            ImGui::SetTooltip("Canvas: %d, %d", pixelX, pixelY);
        }

        ImGui::End();
        ImGui::PopStyleVar();
	}

	void EditorLayer::DrawTools()
	{
        ImGui::Begin("Tools");
        ImGui::Text("Zoom: %.2f", m_ZoomLevel);
        if (ImGui::Button("Reset View")) {
            m_ZoomLevel = 1.0f;
            m_PanOffset = { 0.0f, 0.0f };
        }
        ImGui::End();
	}

	EditorLayer::EditorLayer()
        : Layer("EditorLayer")
	{
	}

	EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::OnImGuiRender()
	{
		// Assume Engine::BeginFrame() is called

		DrawViewport();
		DrawTools();

		// Assume Engine::EndFrame() is called
	}

	void EditorLayer::OnUpdate(float ts)
	{
	}

    void EditorLayer::OnEvent(Event& event)
    {
        // Create a dispatcher to help us check the event type safely
        EventDispatcher dispatcher(event);

        // Check: "Is this a KeyPressedEvent?"
        dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e)
            {
                // If it is, this code runs:
                if (e.GetKeyCode() == GLFW_KEY_TAB)
                {
                    std::cout << "SUCCESS: Tab Key Pressed in EditorLayer!" << std::endl;
                    return true; // Mark as Handled (stops other layers from seeing it)
                }
                return false;
            });
    }
}
