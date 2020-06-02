﻿
#include "Internal.hpp"
#include <LuminoEngine/Rendering/Material.hpp>
#include <LuminoEngine/Scene/Shapes/MeshPrimitiveComponent.hpp>
#include "../SceneManager.hpp"

namespace ln {

//=============================================================================
// PlaneMeshComponent

PlaneMeshComponent::PlaneMeshComponent()
    : m_size(10.0f, 10.0f)
    , m_uvParUnit(0.0f, 0.0f)
{
}

PlaneMeshComponent::~PlaneMeshComponent()
{
}

void PlaneMeshComponent::init()
{
    VisualComponent::init();
}

void PlaneMeshComponent::setMaterial(Material* material)
{
    m_material = material;
}

void PlaneMeshComponent::onRender(RenderingContext* context)
{
    context->setMaterial(m_material);

    auto uv1 = Vector2(
        Math::nearEqual(m_uvParUnit.x, 0.0f) ? 1.0f : m_size.x / m_uvParUnit.x,
        Math::nearEqual(m_uvParUnit.y, 0.0f) ? 1.0f : m_size.y / m_uvParUnit.y);
    
    context->drawPlane(m_size.x, m_size.y, Vector2::Zero, uv1, Color::White);
}

//=============================================================================
// SphereMeshComponent

SphereMeshComponent::SphereMeshComponent()
{
}

SphereMeshComponent::~SphereMeshComponent()
{
}

void SphereMeshComponent::init()
{
    VisualComponent::init();
}

void SphereMeshComponent::setMaterial(Material* material)
{
    m_material = material;
}

void SphereMeshComponent::onRender(RenderingContext* context)
{
    context->setMaterial(m_material);
    context->drawSphere(0.5, 8, 8, Color::White);
}

//=============================================================================
// BoxMeshComponent

BoxMeshComponent::BoxMeshComponent()
{
}

BoxMeshComponent::~BoxMeshComponent()
{
}

void BoxMeshComponent::init()
{
	VisualComponent::init();
	m_material = detail::EngineDomain::sceneManager()->primitiveMeshDefaultMaterial();
	m_box = Box(1.0f);
}

void BoxMeshComponent::setMaterial(Material* material)
{
	m_material = material;
}

void BoxMeshComponent::onRender(RenderingContext* context)
{
	context->setMaterial(m_material);
	context->drawBox(m_box, Color::White);
}

} // namespace ln

