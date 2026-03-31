#pragma once

namespace Yume {

// Event categories
enum class EventCategory {
    None = 0,
    Application = 1 << 0,
    Input = 1 << 1,
    Keyboard = 1 << 2,
    Mouse = 1 << 3,
    MouseButton = 1 << 4,
    Window = 1 << 5
};

// Enhanced event base class
class Event {
public:
    virtual ~Event() = default;

    virtual const char* GetType() const = 0;
    virtual Event* Clone() const = 0;

    // Get the categories this event belongs to
    virtual int GetCategoryFlags() const = 0;

    // Check if event is in category
    bool IsInCategory(EventCategory category) const {
        return GetCategoryFlags() & static_cast<int>(category);
    }
};

// Enhanced macro to define event types with categories
#define DEFINE_EVENT_TYPE_CATEGORY(type, categoryFlags) \
    static const char* GetStaticType() { return #type; } \
    virtual const char* GetType() const override { return GetStaticType(); } \
    virtual Event* Clone() const override { return new type(*this); } \
    virtual int GetCategoryFlags() const override { return categoryFlags; }

// Example event with categories
class KeyPressEvent : public Event {
private:
    int keyCode;
    bool repeat;

public:
    KeyPressEvent(int key, bool isRepeat) : keyCode(key), repeat(isRepeat) {}

    int GetKeyCode() const { return keyCode; }
    bool IsRepeat() const { return repeat; }

    DEFINE_EVENT_TYPE_CATEGORY(KeyPressEvent,
                              static_cast<int>(EventCategory::Input) |
                              static_cast<int>(EventCategory::Keyboard))
};

}