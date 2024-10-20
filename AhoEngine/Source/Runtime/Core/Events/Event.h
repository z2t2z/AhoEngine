#pragma once

#include "Runtime/Core/Core.h"

#include <string>
#include <functional>
#include <queue>

namespace Aho {
	// Event system is currently blocking
	enum class EventType {
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		FileChanged, AssetImported, PackRenderData, UploadRenderData,
		AddEntity,
	};

	enum EventCategory {
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4),
	};

	// ##连接操作符
	// # 将宏参数转化为字符串
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event {
		friend class EventDispatcher;
	public:
		virtual EventType GetEventType() const = 0;
		//virtual EventType GetStaticType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }
		inline bool IsInCategory(EventCategory category) { return GetCategoryFlags() & category; }
		inline bool Handled() { return m_Handled; }
		virtual void SetHandled() { m_Handled = true; }
	protected:
		bool m_Handled = false;
	};

	// TODO: Consider using pointer here...
	class EventDispatcher {
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event) : m_Event(event) {}
		template<typename T>
		bool Dispatch(EventFn<T> func) {
			if (m_Event.GetEventType() == T::GetStaticType()) {
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	class EventManager {
	public:
		EventManager() = default;
		~EventManager() = default;
		bool Empty() { return m_EventQueue.empty(); }
		std::shared_ptr<Event> PopFront() { 
			auto res = GetFront();
			m_EventQueue.pop_front();
			return res; 
		}
		std::shared_ptr<Event> GetFront() { return m_EventQueue.front(); }
		void PushBack(std::shared_ptr<Event> e) { m_EventQueue.push_back(e); }
		int GetQueueSize() { return m_EventQueue.size(); }
	private:
		std::deque<std::shared_ptr<Event>> m_EventQueue;
	};

	class AssetImportedEvent : public Event {
	public:
		AssetImportedEvent(const std::string& filePath) : m_FilePath(filePath) {}
		static EventType GetStaticType() { return EventType::AssetImported; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "AssetImported"; }
		virtual int GetCategoryFlags() const override { return 0; }
		void SetFilePath(const std::string& path) { m_FilePath = path; }
		const std::string& GetFilePath() const { return m_FilePath; }
	private:
		std::string m_FilePath;
	};

	class StaticMesh;
	class PackRenderDataEvent : public Event {
	public:
		PackRenderDataEvent(const std::shared_ptr<StaticMesh>& data)
			: m_Data(data) {
		}
		static EventType GetStaticType() { return EventType::PackRenderData; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "PackRenderData"; }
		virtual int GetCategoryFlags() const override { return 0; }
		std::shared_ptr<StaticMesh> GetRawData() const { return m_Data; }
	private:
		std::shared_ptr<StaticMesh> m_Data{ nullptr };
	};

	class RenderData;
	class UploadRenderDataEvent : public Event {
	public:
		UploadRenderDataEvent(const std::vector<std::shared_ptr<RenderData>>& data)
			: m_RenderData(data) {}
		static EventType GetStaticType() { return EventType::UploadRenderData; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "UploadRenderData"; }
		virtual int GetCategoryFlags() const override { return 0; }
		std::vector<std::shared_ptr<RenderData>> GetRawData() const { return m_RenderData; } // TODO: no copy
	private:
		std::vector<std::shared_ptr<RenderData>> m_RenderData;
	};


	enum class LightType;
	class AddLightSourceEvent : public Event {
	public:
		AddLightSourceEvent(LightType lt)
			: m_LightType(lt) {}
		static EventType GetStaticType() { return EventType::AddEntity; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "AddLight"; }
		virtual int GetCategoryFlags() const override { return 0; }
		LightType GetLightType() const { return m_LightType; } // TODO: no copy
	private:
		LightType m_LightType;
	};
} // namespace Aho