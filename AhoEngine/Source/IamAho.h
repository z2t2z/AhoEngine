#pragma once

// For use by client application
#include "Ahopch.h"
#include <iostream>


#include "Runtime/Core/Parallel.h"
#include "Runtime/Core/IndexAllocator.h"
#include "Runtime/Core/App/Application.h"
#include "Runtime/Core/Layer/Layer.h"
#include "Runtime/Core/Input/Input.h"
#include "Runtime/Core/Input/KeyCodes.h"
#include "Runtime/Core/Input/MouseButtonCodes.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Core/Gui/ImGuiLayer.h"
#include "Runtime/Core/Math/Math.h"
#include "Runtime/Core/Timer.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"

#include "Runtime/Resource/ResourceType/ResourceType.h"
#include "Runtime/Resource/FileWatcher/FileWatcher.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Resource/ResourceManager.h"

#include "Runtime/Function/Renderer/Buffer.h"
#include "Runtime/Function/Renderer/Texture.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
#include "Runtime/Function/Renderer/Material.h"
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Renderer/Framebuffer.h"
#include "Runtime/Function/Renderer/Shader/Shader.h"
#include "Runtime/Function/Renderer/VertexArray.h"
#include "Runtime/Function/Renderer/RendererAPI.h"
#include "Runtime/Function/Renderer/RenderCommand.h"
#include "Runtime/Function/Renderer/RenderLayer.h"
#include "Runtime/Function/Camera/Camera.h"
#include "Runtime/Function/Camera/CameraManager.h"
#include "Runtime/Function/Level/Ecs/Components.h"
#include "Runtime/Function/Level/Ecs/Entity.h"
#include "Runtime/Function/Level/Level.h"
#include "Runtime/Function/Level/LevelLayer.h"