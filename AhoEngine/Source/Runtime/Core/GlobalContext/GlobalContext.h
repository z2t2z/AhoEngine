#pragma once

#include <string>
#include <memory>

namespace Aho {
    class Logger;
    class Renderer;
    class WindowSystem;
    class InputSystem;
    class FileSystem;
    class EntityManager;
    class LevelManager;
    class AssetManager;     // Manage resources from disk
    class ResourceManager;  // Manage runtime resources
    class IBLManager;

    class RuntimeGlobalContext {
    public:
        RuntimeGlobalContext() = default;
        ~RuntimeGlobalContext() = default;
        void InitializeSystems(const std::string& configFilePath);
        void ShutdownSystems();
    public:
        std::shared_ptr<Logger>             m_Logger;
        std::shared_ptr<Renderer>           m_Renderer;
        std::shared_ptr<InputSystem>        m_InputSystem;
        std::shared_ptr<FileSystem>         m_FileSystem;
        std::shared_ptr<AssetManager>       m_AssetManager;
        std::shared_ptr<LevelManager>       m_LevelManager;
        std::shared_ptr<WindowSystem>       m_WindowSystem;
        std::shared_ptr<EntityManager>      m_EntityManager;
        std::shared_ptr<ResourceManager>    m_Resourcemanager;
        std::shared_ptr<IBLManager>         m_IBLManager;
    };

    extern RuntimeGlobalContext g_RuntimeGlobalCtx;

} // namespace Aho
