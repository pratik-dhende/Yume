#pragma once

namespace Yume {

class Event {
public:
	virtual ~Event() = default;
};

class CollisionEvent : public Event {
public:
    CollisionEvent(Entity* entity1, Entity* entity2) : m_entity1(entity1), m_entity2(entity2) {}

    Entity* GetEntity1() const { return m_entity1; }
    Entity* GetEntity2() const { return m_entity2; }

private:
    Entity* m_entity1;
    Entity* m_entity2;
};

class EventListener {
public:
    virtual ~EventListener() = default;
    virtual void OnEvent(const Event& event) = 0;
};

class EventSystem {
public:
    void AddListener(EventListener* listener) {
        m_listeners.push_back(listener);
    }

    void RemoveListener(EventListener* listener) {
        auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
        if (it != m_listeners.end()) {
            m_listeners.erase(it);
        }
    }

    void DispatchEvent(const Event& event) {
        for (auto listener : m_listeners) {
            listener->OnEvent(event);
        }
    }

private:
    std::vector<EventListener*> m_listeners;
};

}