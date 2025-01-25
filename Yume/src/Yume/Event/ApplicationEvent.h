#pragma once

#include "Event.h"

#include <sstream>

namespace Yume
{
	class ApplicationEvent : public Event
	{
	public:
		EVENT_CLASS_CATEGORY(toUType(EventCategory::Application))

	protected:
		ApplicationEvent() { }
	};

	class WindowResizeEvent : public ApplicationEvent
	{
	public:
		EVENT_CLASS_TYPE(WindowResize)

		WindowResizeEvent(const int width, const int height)
			: m_width(width), m_height(height)
		{ }

		int getWidth() const noexcept { return m_width; }
		int getHeight() const noexcept { return m_height; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_width << ", " << m_height;
			return ss.str();
		}

	private:
		int m_width;
		int m_height;
	};

	class WindowCloseEvent : public ApplicationEvent
	{
	public:
		EVENT_CLASS_TYPE(WindowClose)
	};

	class AppTickEvent : public ApplicationEvent
	{
	public:
		EVENT_CLASS_TYPE(AppTick)
	};

	class AppUpdateEvent : public ApplicationEvent
	{
	public:
		EVENT_CLASS_TYPE(AppUpdate)
	};

	class AppRenderEvent : public ApplicationEvent
	{
	public:
		EVENT_CLASS_TYPE(AppRender)
	};
}

