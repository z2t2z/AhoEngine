#pragma once

#include "Event.h"
#include "Runtime/Core/Core.h"
#include <sstream>


namespace Aho {

	class AHO_API WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(unsigned int width, unsigned int height) : m_height(height), m_width(width) {}

		inline unsigned int GetWidth() { return m_width; }
		inline unsigned int GetHeight() { return m_height; }

		std::string ToString() const override {
			std::stringstream ss;  // bad perf
			ss << "WindowResizeEvent: " << m_width << " " << m_height;
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		unsigned int m_width;
		unsigned int m_height;
	};

	class AHO_API WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	// TODO: ApplicTickEvent, Render, ...

}