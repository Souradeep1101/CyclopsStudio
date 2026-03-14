#pragma once

#include "Cyclops/Events/Event.h"
#include "Cyclops/Core/Base.h"

#include <cstdint>

namespace Cyclops
{
	class CYCLOPS_API KeyEvent : public Event
	{
	public:
		inline int32_t GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	protected:
		KeyEvent(int32_t keycode)
			: m_KeyCode(keycode) 
		{
		}

		int32_t m_KeyCode;
	};

	class CYCLOPS_API KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int32_t keycode, int32_t repeatCount)
			: KeyEvent(keycode), m_RepeatCount(repeatCount) 
		{
		}

		inline int32_t GetRepeatCount() const { return m_RepeatCount; }

		// This macro creates: GetStaticType, GetEventType, GetName
		EVENT_CLASS_TYPE(KeyPressed)

	private:
		int32_t m_RepeatCount;
	};

	class CYCLOPS_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int32_t keycode)
			: KeyEvent(keycode) 
		{
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};
}