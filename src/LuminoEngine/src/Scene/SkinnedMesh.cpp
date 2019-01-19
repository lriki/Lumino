﻿
#include "Internal.hpp"
#include <LuminoEngine/Mesh/SkinnedMeshModel.hpp>
#include <LuminoEngine/Visual/SkinnedMeshComponent.hpp>
#include <LuminoEngine/Scene/SkinnedMesh.hpp>
#include "../Mesh/MeshManager.hpp"

namespace ln {

//==============================================================================
// SkinnedMesh

Ref<SkinnedMesh> SkinnedMesh::create()
{
    return newObject<SkinnedMesh>();
}

Ref<SkinnedMesh> SkinnedMesh::create(const StringRef& filePath, float scale)
{
    return newObject<SkinnedMesh>(filePath, scale);
}

SkinnedMesh::SkinnedMesh()
{
}

SkinnedMesh::~SkinnedMesh()
{
}

void SkinnedMesh::init()
{
    VisualObject::init();
    m_component = newObject<SkinnedMeshComponent>();
    addComponent(m_component);
    setMainVisualComponent(m_component);
}

void SkinnedMesh::init(const StringRef& filePath, float scale)
{
    init();
    m_component->setModel(detail::EngineDomain::meshManager()->createSkinnedMeshModel(filePath, scale));
}

SkinnedMeshComponent* SkinnedMesh::staticMeshComponent() const
{
    return m_component;
}

} // namespace ln

