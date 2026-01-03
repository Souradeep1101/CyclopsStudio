#include "Engine.h"
#include "EditorLayer.h"
#include <memory>

int main()
{
	// 1. Boot the Runtime
	// Initializes GLFW, ImGui, and the Renderer context.
	Cyclops::Engine::Init();

	// 2. Mount the Application
	// We push the Editor layer onto the stack. The Engine takes ownership 
	// of this pointer and will call OnAttach(), OnUpdate(), and OnDetach() automatically.
	std::shared_ptr<Cyclops::Layer> m_EditorLayer= std::make_shared<Cyclops::EditorLayer>();
	Cyclops::Engine::PushLayer(m_EditorLayer);

	// 3. Start the Main Loop
	// The Engine now handles the while() loop, time-stepping, and layer iteration.
	Cyclops::Engine::Run();

	// 4. Cleanup
	// Frees all layers and shuts down subsystems.
	Cyclops::Engine::Shutdown();

	return 0;
}