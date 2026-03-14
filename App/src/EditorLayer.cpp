#include "EditorLayer.h"
#include "Cyclops/Core/Engine.h"
#include "Cyclops/Platform/OpenGL/OpenGLDebug.h"
#include "Cyclops/Core/Profiling/Instrumentor.h"
#include "Cyclops/Renderer/Renderer2D.h"
#include "Cyclops/Tools/BrushSerializer.h"
#include "Cyclops/Tools/BrushTool.h"
#include "Cyclops/Events/KeyEvent.h"
#include "Cyclops/Core/Command.h"
#include "Cyclops/Core/CommandHistory.h"
#include "Cyclops/Tools/PaintCommand.h"
#include "Cyclops/Tools/LayerCommands.h" 

#include "Panels/LayerPanel.h" 
#include "Panels/TimelinePanel.h" 
#include "Panels/ViewportPanel.h"

#include "Cyclops/Utils/PlatformUtils.h" 
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h> 
#include <vector> 
#include <chrono> 
#include <GLFW/glfw3.h> 
#include <numeric>

namespace Cyclops
{
    // --- STRESS TEST DATA ---
    struct StressParticle {
        glm::vec2 Position;
        glm::vec2 Velocity;
        glm::vec4 Color;
    };

    static std::vector<StressParticle> s_Particles;
    static bool s_StressTestActive = false;
    static float s_BatchRenderTime = 0.0f;

    // --- GRAPH DATA ---
    static float s_FrameTimeHistory[120] = { 0.0f };
    static int s_HistoryOffset = 0;

    EditorLayer::EditorLayer() : Layer("EditorLayer") {}
    EditorLayer::~EditorLayer() {}

    void EditorLayer::OnAttach()
    {
        int status = gladLoadGLLoader((GLADloadproc)Cyclops::Engine::GetGLProcAddress);
        if (!status) std::cout << "Failed to initialize GLAD in App!" << std::endl;

        m_Canvas = std::make_unique<Canvas>(m_CanvasWidth, m_CanvasHeight);
        m_BrushEngine = std::make_unique<BrushEngine>();

        // Default Brush
        auto softTexture = std::make_shared<Texture2D>("assets/textures/brushes/SoftCircle1.png");
        Brush& brush = m_BrushEngine->GetBrush();
        brush.Type = BrushType::Textured;
        brush.Texture = softTexture;
        brush.Color = { 0.0f, 1.0f, 0.0f, 1.0f };
        brush.Size = 50.0f;
        brush.Spacing = 0.1f;

        m_Tools.push_back(std::make_shared<BrushTool>(m_BrushEngine.get()));
        m_Tools.push_back(std::make_shared<EraserTool>(m_BrushEngine.get()));
        m_ActiveTool = m_Tools[0].get();
    }

    void EditorLayer::OnDetach() {}

    void EditorLayer::OnUpdate(float ts)
    {
        // Update History Graph
        s_FrameTimeHistory[s_HistoryOffset] = ts * 1000.0f; // ms
        s_HistoryOffset = (s_HistoryOffset + 1) % 120;

        // Animation Playback Logic
        if (m_IsPlaying)
        {
            m_TimeAccumulator += ts;
            float frameDuration = 1.0f / (float)m_FrameRate;
            if (m_TimeAccumulator >= frameDuration)
            {
                m_TimeAccumulator -= frameDuration;
                int nextFrame = m_Canvas->GetCurrentFrame() + 1;
                if (nextFrame > m_MaxFrames) nextFrame = 0;
                m_Canvas->SetCurrentFrame(nextFrame);
            }
        }
    }

    // =========================================================
    // RESET PROJECT (Updated to fill White Background)
    // =========================================================
    void EditorLayer::ResetProject()
    {
        m_IsPlaying = false;
        m_CanvasWidth = 800;
        m_CanvasHeight = 600;

        // 1. Reset Core Canvas
        m_Canvas->Resize(m_CanvasWidth, m_CanvasHeight);

        // [CRITICAL FIX] Reserve memory to prevent vector reallocation destroying textures
        auto& layers = m_Canvas->GetLayers();
        layers.clear();
        layers.reserve(64); // Reserve space for 64 layers upfront

        m_Canvas->AddLayer("Background");
        m_Canvas->SetCurrentFrame(0);
        m_MaxFrames = 24;

        m_ViewportPanel.ResetView();

        // 2. Clear Stress Test
        s_StressTestActive = false;
        s_Particles.clear();

        // 3. Initialize Background with White Color
        auto& layer = layers[0]; // Use the reference we got earlier
        layer.CreateKeyframe(0, m_CanvasWidth, m_CanvasHeight);

        auto fbo = layer.Keyframes[0]->Data;
        fbo->Bind();
        GLCall(glViewport(0, 0, m_CanvasWidth, m_CanvasHeight));
        GLCall(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        fbo->Unbind();

        m_Canvas->Recompose();
    }

    // =========================================================
    // IMPORT GENGA SEQUENCE
    // =========================================================
    void EditorLayer::ImportGengaSequence(const std::string& path)
    {
        if (!std::filesystem::exists(path)) return;

        ResetProject();

        std::vector<std::filesystem::path> files;
        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            auto ext = entry.path().extension();
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".PNG")
                files.push_back(entry.path());
        }

        if (files.empty()) return;

        std::sort(files.begin(), files.end());

        // Auto-Resize Canvas
        {
            auto tempTexture = std::make_shared<Texture2D>(files[0].string());
            if (tempTexture->GetWidth() > 0 && tempTexture->GetHeight() > 0)
            {
                m_CanvasWidth = tempTexture->GetWidth();
                m_CanvasHeight = tempTexture->GetHeight();
                m_Canvas->Resize(m_CanvasWidth, m_CanvasHeight);
                m_ViewportPanel.ResetView();
            }
        }

        auto& layer = m_Canvas->GetLayers()[0];
        layer.Name = "Genga Import";

        int frameIdx = 0;
        for (const auto& filePath : files)
        {
            layer.CreateKeyframe(frameIdx, m_CanvasWidth, m_CanvasHeight);
            auto texture = std::make_shared<Texture2D>(filePath.string());
            auto fbo = layer.Keyframes[frameIdx]->Data;

            fbo->Bind();
            GLCall(glViewport(0, 0, m_CanvasWidth, m_CanvasHeight)); // [FIX] Wrapped
            Renderer2D::BeginScene(glm::ortho(0.0f, (float)m_CanvasWidth, 0.0f, (float)m_CanvasHeight));
            Renderer2D::DrawQuad({ 0.0f, 0.0f }, { (float)m_CanvasWidth, (float)m_CanvasHeight }, texture);
            Renderer2D::EndScene();
            fbo->Unbind();

            frameIdx++;
        }

        if (frameIdx > m_MaxFrames) m_MaxFrames = frameIdx - 1;
        m_Canvas->Recompose();
    }

    void EditorLayer::OnImGuiRender()
    {
        Renderer2D::ResetStats();

        // 1. MAIN MENU BAR
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New / Reset Project")) ResetProject();
                ImGui::Separator();
                if (ImGui::MenuItem("Import Genga Sequence..."))
                {
                    std::string folder = FileDialogs::OpenFolder();
                    if (!folder.empty()) ImportGengaSequence(folder);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) Cyclops::Engine::Shutdown();
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // 2. DRAW EDITOR PANELS
        m_ViewportPanel.OnImGuiRender(m_Canvas.get(), m_BrushEngine.get(), m_ActiveTool);
        DrawPanels();

        // 3. PERFORMANCE PANEL (Simplified)
        ImGui::Begin("Performance");
        {
            ImGui::SetWindowFontScale(1.1f);
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "CYCLOPS ENGINE STATS");
            ImGui::Separator();

            // --- VSYNC ---
            // We initialize to TRUE because you enabled it in Engine.cpp
            static bool vsync = true;
            if (ImGui::Checkbox("Use VSync (Lock to Hz)", &vsync))
            {
                Cyclops::Engine::SetVSync(vsync); // Robust SetVSync
            }

            // --- GRAPHS ---
            float fps = ImGui::GetIO().Framerate;
            float avgFrameTime = 1000.0f / fps;

            float maxGraphValue = 0.0f;
            for (float val : s_FrameTimeHistory) {
                if (val > maxGraphValue) maxGraphValue = val;
            }
            if (maxGraphValue < 1.0f) maxGraphValue = 1.0f;

            ImGui::Text("Frame Time History (Max: %.2f ms)", maxGraphValue);
            ImGui::PlotLines("##FrameTime", s_FrameTimeHistory, IM_ARRAYSIZE(s_FrameTimeHistory), s_HistoryOffset,
                nullptr, 0.0f, maxGraphValue * 1.1f, ImVec2(0, 50));

            ImGui::Columns(2, "PerfCols", false);

            ImGui::Text("App FPS:");
            ImGui::NextColumn();
            ImVec4 fpsColor = (fps > 55.0f) ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0.5f, 0, 1);
            ImGui::TextColored(fpsColor, "%.1f", fps);
            ImGui::NextColumn();

            ImGui::Text("Frame Time:");
            ImGui::NextColumn();
            ImGui::Text("%.2f ms", avgFrameTime);
            ImGui::NextColumn();

            // --- BANDWIDTH ---
            if (m_IsPlaying && !vsync) {
                double bandwidth = (m_CanvasWidth * m_CanvasHeight * 4 * fps) / (1024.0 * 1024.0);
                ImGui::Text("Bandwidth:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%.0f MB/s", bandwidth);
            }
            else {
                ImGui::Text("Bandwidth:");
                ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Normal");
            }
            ImGui::NextColumn();

            ImGui::Columns(1);
            ImGui::Separator();

            // --- STRESS TEST ---
            ImGui::TextDisabled("Stress Testing");

            if (!s_StressTestActive)
            {
                if (ImGui::Button("Start 10k Particles", ImVec2(-1, 30)))
                {
                    s_StressTestActive = true;
                    if (s_Particles.empty()) {
                        s_Particles.resize(10000);
                        for (auto& p : s_Particles) {
                            p.Position = { rand() % m_CanvasWidth, rand() % m_CanvasHeight };
                            p.Velocity = { (rand() % 10) - 5.0f, (rand() % 10) - 5.0f };
                            p.Color = { (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1.0f };
                        }
                    }
                }
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button("STOP TEST", ImVec2(-1, 30)))
                {
                    s_StressTestActive = false;
                    s_Particles.clear();
                }
                ImGui::PopStyleColor();

                ImGui::Text("Particle Batch Time: %.3f ms", s_BatchRenderTime);
                if (s_BatchRenderTime > 0.0f)
                    ImGui::TextDisabled("Theoretical FPS: %.0f", 1000.0f / s_BatchRenderTime);
            }

            ImGui::Dummy(ImVec2(0, 10));
            if (ImGui::Button("Reset / Clear Project", ImVec2(-1, 0)))
            {
                ResetProject();
            }
        }
        ImGui::End();

        // =========================================================
        // RENDER LOOP
        // =========================================================
        if (s_StressTestActive)
        {
            auto fbo = m_Canvas->GetActiveFramebuffer();
            if (fbo)
            {
                auto startTime = std::chrono::high_resolution_clock::now();

                fbo->Bind();
                GLCall(glViewport(0, 0, m_CanvasWidth, m_CanvasHeight)); // [FIX] Wrapped
                glm::mat4 cam = glm::ortho(0.0f, (float)m_CanvasWidth, 0.0f, (float)m_CanvasHeight);
                Renderer2D::BeginScene(cam);

                for (auto& p : s_Particles)
                {
                    p.Position += p.Velocity;
                    if (p.Position.x < 0 || p.Position.x > m_CanvasWidth) p.Velocity.x *= -1;
                    if (p.Position.y < 0 || p.Position.y > m_CanvasHeight) p.Velocity.y *= -1;
                    Renderer2D::DrawQuad(p.Position, { 5.0f, 5.0f }, p.Color);
                }

                Renderer2D::EndScene();
                fbo->Unbind();

                auto endTime = std::chrono::high_resolution_clock::now();
                s_BatchRenderTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
                m_Canvas->Recompose();
            }
        }
    }

    void EditorLayer::DrawPanels()
    {
        m_LayerPanel.OnImGuiRender(m_Canvas.get());
        m_TimelinePanel.OnImGuiRender(m_Canvas.get(), m_IsPlaying, m_FrameRate, m_MaxFrames);

        ImGui::Begin("Tools");
        ImGui::Text("Zoom: %.2f", m_ViewportPanel.GetZoomLevel());
        ImGui::Separator();

        Brush& currentBrush = m_BrushEngine->GetBrush();
        ImGui::SliderFloat("Size", &currentBrush.Size, 1.0f, 200.0f);
        ImGui::ColorEdit4("Color", &currentBrush.Color.x);
        ImGui::SliderFloat("Spacing", &currentBrush.Spacing, 0.05f, 1.0f);

        ImGui::Separator();
        auto ToolButton = [&](const char* label, ToolType type) {
            bool isActive = (m_ActiveTool && m_ActiveTool->GetType() == type);
            if (isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
            if (ImGui::Button(label)) {
                for (auto& tool : m_Tools) {
                    if (tool->GetType() == type) { m_ActiveTool = tool.get(); break; }
                }
            }
            if (isActive) ImGui::PopStyleColor();
            ImGui::SameLine();
            };

        ToolButton("Brush", ToolType::Brush);
        ToolButton("Eraser", ToolType::Eraser);

        ImGui::NewLine();
        if (ImGui::Button("Reset View")) m_ViewportPanel.ResetView();
        ImGui::End();
    }

    void EditorLayer::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) {
            bool control = ImGui::GetIO().KeyCtrl;
            if (control && e.GetKeyCode() == GLFW_KEY_Z) { CommandHistory::Undo(); m_Canvas->Recompose(); return true; }
            if (control && e.GetKeyCode() == GLFW_KEY_Y) { CommandHistory::Redo(); m_Canvas->Recompose(); return true; }
            return false;
            });
    }
}