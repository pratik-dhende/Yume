#pragma once

#include <string>
#include <functional>

#include "Yume/API.h"
#include "Yume/Utility/Type.h"

#define SET_BIT(x) 1 << x

namespace Yume
{
	// TODO: Change event system to "buffered" event system in future. (It's "blocking" event system currently)

	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum class EventCategory
	{
		None               = 0,
		Application        = 1 << 1,
		Input              = 1 << 2,
		Keyboard           = 1 << 3,
		Mouse              = 1 << 4,
		MouseButton        = 1 << 5
	};

#define EVENT_CLASS_TYPE(type) static EventType getStaticType() noexcept { return EventType::##type; }\
							   virtual EventType getEventType() const noexcept override { return getStaticType(); }\
							   virtual std::string getName() const noexcept override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const noexcept override { return category; }

	class YM_API Event
	{
	public:
		virtual EventType getEventType() const noexcept = 0;
		virtual std::string getName() const noexcept = 0;
		virtual int getCategoryFlags() const noexcept = 0;

		virtual std::string toString() const noexcept { return getName(); }

		inline bool isInCategory(const EventCategory category) noexcept
		{
			return getCategoryFlags() & toUType(category);
		}

	protected:
		bool m_handled = false;
	};
}