#pragma once

#include "Runtime/Core/Events/Event.h"

namespace Aho {
    class FileChangedEvent : public Event {
    public:
        FileChangedEvent(const std::string& filePath, const std::string& fileType)
            : m_FilePath(filePath), m_FileType(fileType) {
        }
        static EventType GetStaticType() { return EventType::FileChanged; }
        virtual EventType GetEventType() const override { return GetStaticType(); }
        virtual const char* GetName() const override { return "FileChanged"; }
        virtual int GetCategoryFlags() const override { return 0; }
        void SetFilePath(const std::string& path) { m_FilePath = path; }
        const std::string& GetFilePath() const { return m_FilePath; }
        const std::string& GetFileType() const { return m_FileType; }
    private:
        std::string m_FilePath;
        std::string m_FileType;
    };
} // namespace Aho