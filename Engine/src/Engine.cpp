#include "Engine.h"
#include "OpenGLDebug.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Core/PlatformDetection.h"
#include "Debug/Instrumentor.h"

namespace Cyclops
{
	GLFWwindow* Engine::s_Window = nullptr;
    std::vector<std::shared_ptr<Layer>> Engine::s_LayerStack;
    float Engine::s_LastFrameTime = 0.0f;

    // Define the helper function at the bottom of the file
    void Engine::SetEventCallbacks()
    {
        // Set the user pointer so we can access "Engine" context if needed 
        // (Optional for static classes, but good practice for object-oriented ones)
        glfwSetWindowUserPointer(s_Window, nullptr);

        // -- Window Close --
        glfwSetWindowCloseCallback(s_Window, [](GLFWwindow* window)
            {
                WindowCloseEvent event;
                Engine::OnEvent(event);
            });

        // -- Window Resize --
        glfwSetWindowSizeCallback(s_Window, [](GLFWwindow* window, int32_t width, int32_t height)
            {
                WindowResizeEvent event(width, height);
                Engine::OnEvent(event);
            });

        // -- Key Press --
        glfwSetKeyCallback(s_Window, [](GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
            {
                switch (action)
                {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, 0);
                    Engine::OnEvent(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    Engine::OnEvent(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    Engine::OnEvent(event);
                    break;
                }
                }
            });

        // -- Mouse Button --
        glfwSetMouseButtonCallback(s_Window, [](GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
            {
                if (action == GLFW_PRESS)
                {
                    MouseButtonPressedEvent event(button);
                    Engine::OnEvent(event);
                }
                else if (action == GLFW_RELEASE)
                {
                    MouseButtonReleasedEvent event(button);
                    Engine::OnEvent(event);
                }
            });

        // -- Mouse Scroll --
        glfwSetScrollCallback(s_Window, [](GLFWwindow* window, double xOffset, double yOffset)
            {
                MouseScrolledEvent event((float)xOffset, (float)yOffset);
                Engine::OnEvent(event);
            });

        // -- Mouse Move --
        glfwSetCursorPosCallback(s_Window, [](GLFWwindow* window, double xPos, double yPos)
            {
                MouseMovedEvent event((float)xPos, (float)yPos);
                Engine::OnEvent(event);
            });
    }

	void Engine::Init() 
	{
        // Init GLFW
        if (!glfwInit())
            return;
        
        // GL version configs for platform (Windows, Linux)
        const char* glsl_version = "#version 410";
        int gl_major = 4;
        int gl_minor = 6;

        // GL version configs for platform (Mac)
        #if defined(CYCLOPS_PLATFORM_MAC)
            gl_major = 4;
            gl_minor = 1;
            glsl_version = "#version 410";
        #endif

        // Setup GL version
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create a windowed mode window and its OpenGL context
        int32_t width = 1280, height = 720;

        s_Window = glfwCreateWindow(width, height, "Cyclops Studio", NULL, NULL);
        if (!s_Window)
            return;

        // Force window to fill screen
        glfwMaximizeWindow(s_Window);

        // Make the window's context current and enable vsync
        glfwMakeContextCurrent(s_Window);
        glfwSwapInterval(1);

        // Check if GLAD is initialized
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }

        // Enable modern OpenGL debugging for Windows and Linux only, Mac will use the traditional GLCall
        EnableGLDebugging();

        // 2. Init Events (Now just one clean line!)
        SetEventCallbacks();

        // Setup ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // ImGui Configuration
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        // Dark style
        ImGui::StyleColorsDark();

        // When viewports are enabled we tweak WindowRounding/WindowBg
        // so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) 
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup platform/renderer backends
        ImGui_ImplGlfw_InitForOpenGL(s_Window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);        
	}

	void Engine::Shutdown() 
    {
        // Delete layer vector
        for (auto& layer : s_LayerStack) {
            layer->OnDetach();
        }
        s_LayerStack.clear();

        // ImGui shutdown
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // GLFW shutdown
        glfwDestroyWindow(s_Window);
        glfwTerminate();
    }

    void Engine::PushLayer(std::shared_ptr<Layer> layer)
    {
        s_LayerStack.push_back(layer);
        layer->OnAttach(); // Allow the layer to initialize itself
    }

    //void Engine::Run()
    //{
    //    // THE MASTER LOOP
    //    while (!glfwWindowShouldClose(s_Window))
    //    {
    //        // 1. Calculate Delta Time (ts)
    //        float time = (float)glfwGetTime();
    //        float timestep = time - s_LastFrameTime;
    //        s_LastFrameTime = time;

    //        // 2. Update Logic (Physics, Math, AI)
    //        // We loop through the stack and tell every layer to update
    //        for (auto& layer : s_LayerStack)
    //            layer->OnUpdate(timestep);

    //        // 3. Render UI
    //        // We handle the boiler plate Begin/End frame here, 
    //        // so the Layers just focus on drawing widgets.
    //        BeginFrame();

    //        for (auto& layer : s_LayerStack)
    //            layer->OnImGuiRender();

    //        EndFrame();
    //    }
    //}

    void Engine::Run()
    {
        // Start recording to "CyclopsProfile.json"
        CYCLOPS_PROFILE_BEGIN_SESSION("Runtime", "CyclopsProfile.json");

        while (!glfwWindowShouldClose(s_Window))
        {
            // Measure the entire frame
            CYCLOPS_PROFILE_SCOPE("RunLoop");

            float time = (float)glfwGetTime();
            float timestep = time - s_LastFrameTime;
            s_LastFrameTime = time;

            {
                // Measure just the Update phase
                CYCLOPS_PROFILE_SCOPE("LayerStack Updates");
                for (auto& layer : s_LayerStack)
                    layer->OnUpdate(timestep);
            }

            {
                // Measure UI Rendering
                CYCLOPS_PROFILE_SCOPE("LayerStack ImGui");
                BeginFrame();
                for (auto& layer : s_LayerStack)
                    layer->OnImGuiRender();
                EndFrame();
            }
        }

        // Stop recording
        CYCLOPS_PROFILE_END_SESSION();
    }

    // 1. THE RECEPTIONIST FUNCTION
    void Engine::OnEvent(Event& e)
    {
        // 1. Print for debugging (Optional, remove later)
         std::cout << "Event: " << e.GetName() << std::endl;

        // 2. The Loop: Top (End) -> Bottom (Begin)
        // We iterate backwards so the UI gets the event first!
        for (auto it = s_LayerStack.rbegin(); it != s_LayerStack.rend(); ++it)
        {
            if (e.Handled)
                break; // If a layer handled it, stop passing it down!

            (*it)->OnEvent(e);
        }
    }

	void Engine::BeginFrame() 
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create the Dockspace (The background that holds other windows)
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
    }

	void Engine::EndFrame() 
    {
        // Render ImGui
        ImGui::Render();

        int32_t display_w, display_h;
        glfwGetFramebufferSize(s_Window, &display_w, &display_h);
        GLCall(glViewport(0, 0, display_w, display_h));

        // Clear screen
        GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        // Draw ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and render additional platform windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        // Swap buffer
        glfwSwapBuffers(s_Window);
    }
}