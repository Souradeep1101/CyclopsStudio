#pragma once

#include "Cyclops/Events/Event.h"
#include "Cyclops/Core/Base.h"

#include <string>

namespace Cyclops
{
    /**
     * @class Layer
     * @brief Base class for any system that needs to be updated or receive events.
     * * @details Layers are stored in the Engine's LayerStack.
     * Examples of Layers: "GameLayer" (Updates physics), "ImGuiLayer" (Draws UI), "DebugLayer".
     * This class uses "Virtual Default" functions, so you only need to override what you use.
     */
	class CYCLOPS_API Layer
	{
	public:
        /**
         * @brief Construct a new Layer.
         * @param name Debug name for profiling and identification.
         */
        Layer(const std::string& name = "Layer")
            : m_DebugName{ name }
        {
        }

        /// @brief Virtual destructor to ensure derived classes clean up correctly.
        virtual ~Layer() = default;

        // Disable Copying: Layers represent unique systems and should not be duplicated.
        Layer(const Layer&) = delete;
        Layer& operator=(const Layer&) = delete;

        /**
         * @brief Called when the Layer is pushed to the Engine stack.
         * @note Use this for initialization instead of the constructor.
         */
        virtual void OnAttach() {}

        /**
         * @brief Called when the Layer is removed or the App shuts down.
         * @note Free your resources here.
         */
        virtual void OnDetach() {}

        /**
         * @brief Called once per frame during the Logic Phase.
         * @param ts Timestep (Delta Time) in seconds.
         */
        virtual void OnUpdate(float ts) {}

        /**
         * @brief Called once per frame during the UI Phase.
         * @note All ImGui::Begin()/End() calls must happen inside this function.
         */
        virtual void OnImGuiRender() {}

        /**
         * @brief Called whenever an event occurs (Mouse, Keyboard, Window).
         * @param event The event to handle.
         */
        virtual void OnEvent(Event& event) {}

        // --- Getters ---

        /// @return The debug name of the layer.
        const std::string& GetName() const { return m_DebugName; }

    protected:
        std::string m_DebugName;
	};
}
