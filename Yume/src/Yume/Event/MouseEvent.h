#pragma once

#include "Event.h"

#include <sstream>

namespace Yume
{	
	class MouseEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(toUType(EventCategory::Mouse) | toUType(EventCategory::Input));

	protected:
		MouseEvent() {}
	};

	class MouseMovedEvent : public MouseEvent
	{
	public:
		EVENT_CLASS_TYPE(MouseMoved)

		MouseMovedEvent(const float x, const float y, const bool delta)
			: m_mouseX(x), m_mouseY(y), m_delta(delta)
		{ }

		float getX() const noexcept { return m_mouseX; }
		float getY() const noexcept { return m_mouseY; }

		bool isDelta() const noexcept { return m_delta; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_mouseX << ", " << m_mouseY;
			return ss.str();
		}

	private:
		float m_mouseX;
		float m_mouseY;
		bool m_delta;
	};

	class MouseScrolledEvent : public MouseEvent
	{
	public:
		EVENT_CLASS_TYPE(MouseScrolled)

		MouseScrolledEvent(const float xOffset, const float yOffset)
			: m_xOffset(xOffset), m_yOffset(yOffset) 
		{ }

		float getXOffset() const noexcept { return m_xOffset; }
		float getYOffset() const noexcept { return m_yOffset; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << getXOffset() << ", " << getYOffset();
			return ss.str();
		}
	private:
		float m_xOffset;
		float m_yOffset;
	};

	class MouseButtonEvent : public MouseEvent
	{
	public:
		int getMouseButton() const noexcept { return m_button; }

	protected:
		MouseButtonEvent(int button)
			: m_button(button)
		{ }

	protected:
		int m_button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		EVENT_CLASS_TYPE(MouseButtonPressed)

		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button)
		{ }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent " << m_button;
			return ss.str();
		}
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		EVENT_CLASS_TYPE(MouseButtonReleased)

		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) 
		{ }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent " << m_button;
			return ss.str();
		}
	};
}

