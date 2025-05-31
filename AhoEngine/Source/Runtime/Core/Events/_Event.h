#pragma once
#include <functional>
#include <vector>


namespace Aho {
    template<typename... Args>
    class _Event {
    public:
        using Listener = std::function<void(Args...)>;

        void AddListener(Listener listener) {
            m_Listeners.push_back(std::move(listener));
        }

        void Dispatch(Args... args) {
            for (auto& listener : m_Listeners) {
                listener(args...);
            }
        }

    private:
        std::vector<Listener> m_Listeners;
    };
}
