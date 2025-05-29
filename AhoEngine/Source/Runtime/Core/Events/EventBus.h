#pragma once

#include "_Event.h"
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cassert>

namespace Aho {
    // what for?
    class EventBus {
    public:
        template<typename EventType>
        Event<EventType>& Get() {
            std::type_index key = std::type_index(typeid(Event<EventType>));
            if (m_Events.find(key) == m_Events.end()) {
                m_Events[key] = std::make_unique<Event<EventType>>();
            }
            return *static_cast<Event<EventType>*>(m_Events[key].get());
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<void>> m_Events;
    };
}
