#pragma once

#include "Event.h"

#include <sstream>

namespace Yume
{
	class KeyEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(toUType(EventCategory::Keyboard) | toUType(EventCategory::Input))

		int getKeyCode() const noexcept { return m_keyCode; };

	protected:
		KeyEvent(const int keyCode)
			: m_keyCode(keyCode) 
		{ }

		int m_keyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		EVENT_CLASS_TYPE(KeyPressed)

		KeyPressedEvent(const int keyCode, const bool isRepeat)
			: KeyEvent(keyCode), m_isRepeat(isRepeat) 
		{ }

		bool isRepeat() const noexcept{ return m_isRepeat; }

		std::string toString() const override
		{	
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_keyCode << " (repeat = " << m_isRepeat << ")";
			return ss.str();
		}

	private:
		bool m_isRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		EVENT_CLASS_TYPE(KeyReleased)

		KeyReleasedEvent(const int keyCode)
			: KeyEvent(keyCode) 
		{ }

		std::string toString() const override
		{	
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_keyCode;
			return ss.str();
		}
	};
}