﻿
#include "Internal.hpp"
#include "../Engine/RenderingCommandList.hpp"
#include "GraphicsManager.hpp"
#include "GraphicsDeviceContext.hpp"
#include <LuminoEngine/Graphics/IndexBuffer.hpp>

namespace ln {

//==============================================================================
// IndexBuffer

Ref<IndexBuffer> IndexBuffer::create(int indexCount, IndexBufferFormat format, GraphicsResourceUsage usage)
{
    return newObject<IndexBuffer>(indexCount, format, usage);
}

Ref<IndexBuffer> IndexBuffer::create(int indexCount, IndexBufferFormat format, const void* initialData, GraphicsResourceUsage usage)
{
    return newObject<IndexBuffer>(indexCount, format, initialData, usage);
}

IndexBuffer::IndexBuffer()
    : m_rhiObject(nullptr)
    , m_format(IndexBufferFormat::UInt16)
    , m_usage(GraphicsResourceUsage::Static)
    , m_pool(GraphicsResourcePool::Managed)
    , m_primaryIndexCount(0)
    , m_buffer()
    , m_rhiMappedBuffer(nullptr)
    , m_mappedBuffer(nullptr)
    , m_initialUpdate(true)
    , m_modified(false)
{
}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::init(int indexCount, IndexBufferFormat format, GraphicsResourceUsage usage)
{
    GraphicsResource::init();
    m_format = format;
    m_usage = usage;
    m_modified = true;
    resize(indexCount);
}

void IndexBuffer::init(int indexCount, IndexBufferFormat format, const void* initialData, GraphicsResourceUsage usage)
{
    IndexBuffer::init(indexCount, format, usage);
    if (initialData) {
        m_rhiObject = detail::GraphicsResourceInternal::manager(this)->deviceContext()->createIndexBuffer(m_usage, m_format, indexCount, initialData);
        m_modified = false;
    }
}

void IndexBuffer::onDispose(bool explicitDisposing)
{
    m_rhiObject.reset();
    GraphicsResource::onDispose(explicitDisposing);
}

int IndexBuffer::size() const
{
    return m_primaryIndexCount;
}

int IndexBuffer::bytesSize() const
{
    return m_primaryIndexCount * getIndexStride();
}

void IndexBuffer::reserve(int indexCount)
{
    m_buffer.reserve(static_cast<size_t>(indexCount * getIndexStride()));
}

void IndexBuffer::resize(int indexCount)
{
    m_primaryIndexCount = indexCount;
    m_buffer.resize(static_cast<size_t>(indexCount * getIndexStride()));
}

void* IndexBuffer::map(MapMode mode)
{
    if (m_mappedBuffer) {
        return m_mappedBuffer;
    }

    // if have not entried the Command List at least once, can rewrite directly with map().
    if (m_initialUpdate && m_usage == GraphicsResourceUsage::Static && m_pool == GraphicsResourcePool::None) {
        if (!m_rhiObject) {
            m_rhiObject = detail::GraphicsResourceInternal::manager(this)->deviceContext()->createIndexBuffer(m_usage, m_format, size(), nullptr);
        }

        if (m_rhiMappedBuffer == nullptr) {
            m_rhiMappedBuffer = detail::GraphicsResourceInternal::manager(this)->deviceContext()->getGraphicsContext()->map(m_rhiObject);
        }

        m_modified = true;
        m_mappedBuffer = m_rhiMappedBuffer;
    } else {
        // prepare for GraphicsResourcePool::None
        size_t primarySize = bytesSize();
        if (m_buffer.size() < primarySize) {
            m_buffer.resize(primarySize);
        }

        m_modified = true;
        m_mappedBuffer = m_buffer.data();
    }

    return m_mappedBuffer;
}

void IndexBuffer::clear()
{
    if (LN_REQUIRE(m_usage == GraphicsResourceUsage::Dynamic)) return;
    m_buffer.clear();
    m_primaryIndexCount = 0;
    m_modified = true;
}

void IndexBuffer::setFormat(IndexBufferFormat format)
{
    size_t indexCount = size();
    IndexBufferFormat oldFormat = m_format;
    m_format = format;

    if (indexCount > 0) {
        if (oldFormat == IndexBufferFormat::UInt16 && m_format == IndexBufferFormat::UInt32) {
            // 16 -> 32
            m_buffer.resize(indexCount * sizeof(uint32_t));
            auto* rpos16 = (uint16_t*)(m_buffer.data() + ((indexCount - 1) * sizeof(uint16_t)));
            auto* rend16 = (uint16_t*)(m_buffer.data());
            auto* rpos32 = (uint32_t*)(m_buffer.data() + ((indexCount - 1) * sizeof(uint32_t)));
            auto* rend32 = (uint32_t*)(m_buffer.data());
            for (; rpos32 >= rend32; rpos32--, rpos16--) {
                uint16_t t = *rpos16;
                *rpos32 = t;
            }
        } else if (oldFormat == IndexBufferFormat::UInt32 && m_format == IndexBufferFormat::UInt16) {
            LN_NOTIMPLEMENTED();
        }
    }
}

void IndexBuffer::setIndex(int index, int vertexIndex)
{
    void* indexBuffer = map(MapMode::Write);

    if (m_format == IndexBufferFormat::UInt16) {
        uint16_t* i = (uint16_t*)indexBuffer;
        i[index] = vertexIndex;
    } else if (m_format == IndexBufferFormat::UInt32) {
        uint32_t* i = (uint32_t*)indexBuffer;
        i[index] = vertexIndex;
    } else {
        LN_NOTIMPLEMENTED();
    }
}

int IndexBuffer::index(int index)
{
    void* indexBuffer = map(MapMode::Read);

    if (m_format == IndexBufferFormat::UInt16) {
        uint16_t* i = (uint16_t*)indexBuffer;
        return i[index];
    } else if (m_format == IndexBufferFormat::UInt32) {
        uint32_t* i = (uint32_t*)indexBuffer;
        return i[index];
    } else {
        LN_NOTIMPLEMENTED();
        return 0;
    }
}

void IndexBuffer::setResourceUsage(GraphicsResourceUsage usage)
{
    // Prohibit while direct locking.
    if (LN_REQUIRE(!m_rhiMappedBuffer)) return;
    if (m_usage != usage) {
        m_usage = usage;
        m_modified = true;
    }
}

void IndexBuffer::setResourcePool(GraphicsResourcePool pool)
{
    m_pool = pool;
}

detail::IIndexBuffer* IndexBuffer::resolveRHIObject(bool* outModified)
{
	*outModified = m_modified;
    m_mappedBuffer = nullptr;

    if (m_modified) {
        detail::IGraphicsDevice* device = detail::GraphicsResourceInternal::manager(this)->deviceContext();
        if (m_rhiMappedBuffer) {
            device->getGraphicsContext()->unmap(m_rhiObject);
            m_rhiMappedBuffer = nullptr;
        } else {
            size_t requiredSize = bytesSize();
            if (!m_rhiObject || m_rhiObject->getBytesSize() != requiredSize || m_rhiObject->usage() != m_usage) {
                m_rhiObject = device->createIndexBuffer(m_usage, m_format, size(), m_buffer.data());
            } else {
                detail::RenderBulkData data(m_buffer.data(), m_buffer.size());
                detail::IIndexBuffer* rhiObject = m_rhiObject;
                LN_ENQUEUE_RENDER_COMMAND_3(
                    IndexBuffer_setSubData, detail::GraphicsResourceInternal::manager(this), detail::IGraphicsDevice*, device, detail::RenderBulkData, data, Ref<detail::IIndexBuffer>, rhiObject, {
                        device->getGraphicsContext()->setSubData(rhiObject, 0, data.data(), data.size());
                    });
            }
        }
    }

    if (LN_ENSURE(m_rhiObject)) return nullptr;

    if (m_usage == GraphicsResourceUsage::Static && m_pool == GraphicsResourcePool::None) {
        m_buffer.clear();
        m_buffer.shrink_to_fit();
    }

    m_initialUpdate = false;
    m_modified = false;
    return m_rhiObject;
}

int IndexBuffer::getIndexStride() const
{
    return getIndexStride(m_format);
}

void IndexBuffer::onChangeDevice(detail::IGraphicsDevice* device)
{
    if (device) {
        if (m_pool == GraphicsResourcePool::Managed) {
            // data is transferred by the next resolveRHIObject()
            m_modified = true;
        }
    } else {
        m_rhiObject.reset();
    }
}

} // namespace ln
