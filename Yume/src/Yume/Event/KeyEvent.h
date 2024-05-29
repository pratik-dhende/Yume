#pragma once

#include "Event.h"

namespace Yume
{
	class YM_API KeyEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(toUType(EventCategory::Keyboard) | toUType(EventCategory::Input))

		inline int getKeyCode() const noexcept { return m_keyCode; };

	protected:
		KeyEvent(const int keyCode)
			: m_keyCode(keyCode) { }

		int m_keyCode;
	};

	class YM_API KeyPressedEvent : public KeyEvent
	{
	public:
		EVENT_CLASS_TYPE(KeyPressed)

		KeyPressedEvent(const int keyCode, const bool isRepeat)
			: KeyEvent(keyCode), m_isRepeat(isRepeat) { }

		inline bool isRepeat() const noexcept{ return m_isRepeat; }

		std::string toString() const noexcept override
		{
			return "KeyPressedEvent: " + std::to_string(m_keyCode) + " (repeat = " + std::to_string(m_isRepeat) + ")";
		}

	private:
		bool m_isRepeat;
	};

	class YM_API KeyReleasedEvent : public KeyEvent
	{
	public:
		EVENT_CLASS_TYPE(KeyReleased)

		KeyReleasedEvent(const int keyCode)
			: KeyEvent(keyCode) { }

		std::string toString()
		{
			return "KeyReleasedEvent: " + std::to_string(m_keyCode);
		}
	};
}