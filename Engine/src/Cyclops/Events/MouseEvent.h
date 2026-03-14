#pragma once

#include "Cyclops/Events/Event.h"
#include "Cyclops/Core/Base.h"

#include <cstdint>

namespace Cyclops
{
	class CYCLOPS_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			: m_MouseX(x), m_MouseY(y) 
		{
		}

		inline float GetX() const { return m_MouseX; }
		inline float GetY() const { return m_MouseY; }

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float m_MouseX, m_MouseY;
	};

	class CYCLOPS_API MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) 
		{
		}

		inline float GetXOffset() const { return m_XOffset; }
		inline float GetYOffset() const { return m_YOffset; }

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
		float m_XOffset, m_YOffset;
	};

	class CYCLOPS_API MouseButtonEvent : public Event
	{
	public:
		inline int32_t GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	protected:
		MouseButtonEvent(int32_t button)
			: m_Button(button) 
		{
		}

		int32_t m_Button;
	};

	class CYCLOPS_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int32_t button)
			: MouseButtonEvent(button) 
		{
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class CYCLOPS_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int32_t button)
			: MouseButtonEvent(button) 
		{
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}