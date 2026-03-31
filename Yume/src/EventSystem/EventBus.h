#pragma once

#include "EventListener.h"
#include "Event.h"

#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <algorithm>

namespace Yume {

class EventBus {
private:
    struct ListenerInfo {
        EventListener* listener;
        int categoryFilter;
        int priority;
    };

    std::vector<ListenerInfo> m_listeners;
    std::queue<std::unique_ptr<Event>> m_eventQueue;
    std::mutex m_eventQueueMutex;
    bool m_immediateMode = true;

public:
    void AddListener(EventListener* listener, int categoryFilter = -1, int priority = 0) {
        m_listeners.push_back({listener, categoryFilter, priority});

        // Sort listeners by priority (higher priority first)
        std::sort(m_listeners.begin(), m_listeners.end(),
                 [](const ListenerInfo& a, const ListenerInfo& b) {
                     return a.priority > b.priority;
                 });
    }

    void RemoveListener(EventListener* listener) {
        auto it = std::find_if(m_listeners.begin(), m_listeners.end(),
                              [listener](const ListenerInfo& info) {
                                  return info.listener == listener;
                              });
        if (it != m_listeners.end()) {
            m_listeners.erase(it);
        }
    }

    void PublishEvent(const Event& event) {
        if (m_immediateMode) {
            // Dispatch event immediately
            for (const auto& info : m_listeners) {
                if (info.categoryFilter == -1 || (event.GetCategoryFlags() & info.categoryFilter)) {
                    info.listener->OnEvent(event);
                }
            }
        } else {
            // Queue event for later processing
            std::lock_guard<std::mutex> lock(m_eventQueueMutex);
            m_eventQueue.push(std::unique_ptr<Event>(event.Clone()));
        }
    }

    void ProcessEvents() {
        if (m_immediateMode) return;

        std::queue<std::unique_ptr<Event>> currentEvents;

        {
            std::lock_guard<std::mutex> lock(m_eventQueueMutex);
            std::swap(currentEvents, m_eventQueue);
        }

        while (!currentEvents.empty()) {
            auto& event = *currentEvents.front();

            for (const auto& info : m_listeners) {
                info.listener->OnEvent(event);
            }

            currentEvents.pop();
        }
    }
};

}