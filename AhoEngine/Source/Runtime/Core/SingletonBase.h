#pragma once

#include <memory>
#include <mutex>

namespace Aho {
    template <typename T>
    class SingletonBase {
    public:
        static T& getInstance() {
            static std::once_flag flag;
            std::call_once(flag,
                []() {
                    m_Instance.reset(new T());
                });
            return *m_Instance;
        }

        SingletonBase(const SingletonBase&) = delete;
        SingletonBase& operator=(const SingletonBase&) = delete;

    protected:
        SingletonBase() = default;
        virtual ~SingletonBase() = default;

    private:
        static std::unique_ptr<T> m_Instance;
    };
}

