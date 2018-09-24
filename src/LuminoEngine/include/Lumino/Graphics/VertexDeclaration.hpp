﻿
#pragma once
#include "Common.hpp"
#include "GraphicsResource.hpp"

namespace ln {
namespace detail { class IVertexDeclaration; }

class VertexDeclaration
	: public GraphicsResource
{
public:
	static Ref<VertexDeclaration> create();
	
	void addVertexElement(int streamIndex, VertexElementType type, VertexElementUsage usage, int usageIndex);

protected:
	virtual void dispose() override;

LN_CONSTRUCT_ACCESS:
	VertexDeclaration();
	virtual ~VertexDeclaration();
	void initialize();
	void initialize(const VertexElement* elements, int count);

LN_INTERNAL_ACCESS:
	detail::IVertexDeclaration* resolveRHIObject();

LN_PROTECTED_INTERNAL_ACCESS:
	virtual void onChangeDevice(detail::IGraphicsDeviceContext* device) override;

private:
	Ref<detail::IVertexDeclaration>	m_deviceObj;
	List<VertexElement> m_vertexElements;
	bool m_modified;
};

} // namespace ln