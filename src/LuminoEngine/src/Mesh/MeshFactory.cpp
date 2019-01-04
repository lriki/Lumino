﻿
#include "Internal.hpp"
#include <LuminoEngine/Rendering/Vertex.hpp>
#include "MeshFactory.hpp"

namespace ln {
namespace detail {

void MeshGeneraterBuffer::setBuffer(Vertex* vertexBuffer, void* indexBuffer, IndexBufferFormat indexFormat, uint32_t indexNumberOffset)
{
    m_vertexBuffer = vertexBuffer;
    m_indexBuffer = indexBuffer;
    m_indexFormat = indexFormat;
    m_indexNumberOffset = indexNumberOffset;
}

} // namespace detail
} // namespace ln

