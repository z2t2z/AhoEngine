#include "Ahopch.h"
#include "AObject.h"

namespace Aho {
	AObject::AObject(entt::entity handle, Scene* scene, const UUID& uuid) : m_AObjectHandle(handle), m_Scene(scene), m_AssetUUID(uuid) {

	}
	AObject::AObject(entt::entity handle, Scene* scene) : m_AObjectHandle(handle), m_Scene(scene) {

	}
}