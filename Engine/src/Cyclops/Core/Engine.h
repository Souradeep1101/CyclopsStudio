#pragma once

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Cyclops/Core/Layer.h"
#include "Cyclops/Core/Base.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>

namespace Cyclops
{
	/**
	 * @class Engine
	 * @brief The static Singleton-like Application class that manages the main loop and system lifecycle.
	 * * @details The Engine is responsible for:
	 * - Initializing core systems (GLFW, ImGui, OpenGL).
	 * - Managing the Window and Context.
	 * - Holding the LayerStack (the ordered list of application layers).
	 * - Dispatching Events to Layers.
	 * - Running the main "Game Loop" (Update -> Render).
	 */
	class CYCLOPS_API Engine
	{
	private:
		/// @brief The native GLFW window handle.
		static GLFWwindow* s_Window;

		/// @brief The stack of Layers (Tools, Game World, UI).
		/// @note We use shared_ptr so layers can be safely shared or referenced by other systems if needed.
		static std::vector<std::shared_ptr<Layer>> s_LayerStack;

		/// @brief Timestamp of the previous frame (used to calculate Delta Time).
		static float s_LastFrameTime;

		/// @brief Configures internal GLFW callbacks (Key press, Mouse move, etc.).
		static void SetEventCallbacks();
		
	public:
		/**
		 * @brief Initializes the Engine core.
		 * @details Sets up GLFW, creates the Window, loads OpenGL (GLAD), and configures ImGui.
		 * Must be called before Run().
		 */
		static void Init();

		/**
		 * @brief Shuts down the Engine and frees all resources.
		 * @details Clears the LayerStack, destroys the ImGui context, and terminates GLFW.
		 */
		static void Shutdown();

		/**
		 * @brief The Master Loop.
		 * @details Runs continuously until the window is closed.
		 * Cycle:
		 * 1. Calculate Delta Time.
		 * 2. Layer::OnUpdate() (Logic).
		 * 3. Layer::OnImGuiRender() (UI).
		 * 4. Swap Buffers.
		 */
		static void Run();

		/**
		 * @brief Pushes a new Layer onto the stack.
		 * @param layer A shared pointer to the Layer. Use std::make_shared<MyLayer>() to create.
		 * @details The Engine takes shared ownership of the layer and immediately calls OnAttach().
		 */
		static void PushLayer(std::shared_ptr<Layer> layer);

		/**
		 * @brief The Central Event Dispatcher.
		 * @param e The generic Event reference.
		 * @details Propagates the event backwards through the LayerStack (Overlay -> Bottom).
		 * If a Layer sets e.Handled = true, propagation stops.
		 */
		static void OnEvent(Event& e);

		/**
		 * @brief Prepares the ImGui and OpenGL context for a new frame.
		 */
		static void BeginFrame();

		/**
		 * @brief Finalizes rendering, draws ImGui data, and swaps buffers.
		 */
		static void EndFrame();

		// Helper to let the App use the Engine's GLFW context
		static void* GetGLProcAddress(const char* name);

		// Set VSync
		static void SetVSync(bool enabled);
		
		// --- Getters ---

		/** * @return The raw GLFW window pointer. Useful for low-level GLFW queries.
		 */
		static GLFWwindow* GetWindow() { return s_Window; }
	};
}