﻿
#include "Internal.hpp"
#include <LuminoEngine/Physics/PhysicsWorld.hpp>
#include <LuminoEngine/Physics/PhysicsWorld2D.hpp>
#include "PhysicsManager.hpp"

namespace ln {
namespace detail {

//==============================================================================
// PhysicsManager

PhysicsManager::PhysicsManager()
{
}

PhysicsManager::~PhysicsManager()
{
}

void PhysicsManager::init(const Settings& settings)
{
}

void PhysicsManager::dispose()
{
}

void PhysicsManager::setActivePhysicsWorld2D(PhysicsWorld2D* value)
{
    m_activePhysicsWorld2D = value;
}

const Ref<PhysicsWorld2D>& PhysicsManager::activePhysicsWorld2D() const
{
    return m_activePhysicsWorld2D;
}

} // namespace detail
} // namespace ln

