#pragma once

#include <string>
#include <functional>
#include <iostream>
#include <cstdint>

namespace Cyclops
{
	/**
	 * @enum EventType
	 * @brief Unique identifiers for every distinct event supported by the engine.
	 */
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	/**
	 * @enum EventCategory
	 * @brief Bitfield flags to filter events by group (e.g., "Input", "Mouse").
	 * @details Allows checking if an event belongs to multiple categories
	 * (e.g., a MouseButton event is both "Input" and "Mouse").
	 */
	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = (1 << 0),
		EventCategoryInput = (1 << 1),
		EventCategoryKeyboard = (1 << 2),
		EventCategoryMouse = (1 << 3),
		EventCategoryMouseButton = (1 << 4)
	};

// Macros to generate boilerplate code automatically
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int32_t GetCategoryFlags() const override { return category; }

	/**
	 * @class Event
	 * @brief The base class for the Event System.
	 * @details Events in Cyclops are currently blocking. When an event occurs,
	 * it is immediately dispatched and must be handled in the current frame.
	 */
	class Event
	{
	public:
		virtual ~Event() = default;

		/// @brief Flag indicating if the event has been consumed by a layer.
		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int32_t GetCategoryFlags() const = 0;

		/// @brief Fast bitwise check to see if an event falls into a specific category.
		bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
	};

	/**
	 * @class EventDispatcher
	 * @brief Utility for safely dispatching events based on their runtime type.
	 * @details Uses C++ templates to cast the generic Event& to a specific type (e.g., KeyPressedEvent&)
	 * only if the types match.
	 */
	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}

		/**
		 * @brief Dispatches the event if it matches the template type T.
		 * @tparam T The specific Event class to check for (e.g., KeyPressedEvent).
		 * @tparam F The lambda function to call if the match is successful.
		 * @return True if the event matched the type, False otherwise.
		 */
		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				// If the function returns true (Handled), we update the event's state.
				m_Event.Handled |= func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};
}
