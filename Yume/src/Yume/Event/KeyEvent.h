#pragma once

#include <sstream>
#include "Event.h"

namespace Yume
{
	class YM_API KeyEvent : public Event
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

	class YM_API KeyPressedEvent : public KeyEvent
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

	class YM_API KeyReleasedEvent : public KeyEvent
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