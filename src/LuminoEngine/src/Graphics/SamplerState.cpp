﻿
#include "Internal.hpp"
#include <LuminoEngine/Graphics/SamplerState.hpp>
#include "GraphicsDeviceContext.hpp"
#include "GraphicsManager.hpp"

namespace ln {

const SamplerStateData SamplerStateData::defaultState =
{
	TextureFilterMode::Point,
	TextureAddressMode::Repeat,
};

//=============================================================================
// SamplerState

SamplerState::SamplerState()
	: m_rhiObject(nullptr)
	, m_desc(SamplerStateData::defaultState)
	, m_modified(true)
	, m_frozen(false)
{
}

SamplerState::~SamplerState()
{
}

void SamplerState::initialize()
{
	GraphicsResource::initialize();
}

void SamplerState::dispose()
{
	m_rhiObject.reset();
	GraphicsResource::dispose();
}

void SamplerState::setFilterMode(TextureFilterMode value)
{
	if (LN_REQUIRE(!m_frozen)) return;
	if (m_desc.filter != value)
	{
		m_desc.filter = value;
		m_modified = true;
	}
}

void SamplerState::setAddressMode(TextureAddressMode value)
{
	if (LN_REQUIRE(!m_frozen)) return;
	if (m_desc.address != value)
	{
		m_desc.address = value;
		m_modified = true;
	}
}

detail::ISamplerState* SamplerState::resolveRHIObject()
{
	if (m_modified)
	{
		m_rhiObject = deviceContext()->createSamplerState(m_desc);
		m_modified = false;
	}

	return m_rhiObject;
}

void SamplerState::onChangeDevice(detail::IGraphicsDeviceContext* device)
{
	if (device)
	{
		m_modified = true;
	}
	else
	{
		m_rhiObject.reset();
	}
}

} // namespace ln
