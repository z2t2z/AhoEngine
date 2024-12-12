#pragma once

#include "Core.h"
#include "Log/Log.h"

#include <chrono>
#include <string>
#include <iostream>

namespace Aho {
    class Timer {
    public:
        Timer() : m_StartTime(std::chrono::high_resolution_clock::now()) {}

        void Reset() {
            m_StartTime = std::chrono::high_resolution_clock::now();
        }

        double ElapsedMilliseconds() const {
            auto currentTime = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double, std::milli>(currentTime - m_StartTime).count();
        }

        double ElapsedSeconds() const {
            auto currentTime = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double>(currentTime - m_StartTime).count();
        }

    private:
        std::chrono::high_resolution_clock::time_point m_StartTime;
    };

    class ScopedTimer {
    public:
        ScopedTimer(const std::string& name) : m_Name(name), startTime(std::chrono::high_resolution_clock::now()) {}

        ~ScopedTimer() {
            auto endTime = std::chrono::high_resolution_clock::now();
            double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            AHO_CORE_INFO("{} took {} ms", m_Name, elapsedMs);
        }

    private:
        std::string m_Name;
        std::chrono::high_resolution_clock::time_point startTime;
    };

}