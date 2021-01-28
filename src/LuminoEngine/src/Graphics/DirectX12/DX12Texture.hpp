﻿
#pragma once
#include "DX12DeviceContext.hpp"

namespace ln {
namespace detail {
	
class DX12RenderTarget
    : public DX12Texture
{
public:
    DX12RenderTarget();
    bool init(DX12Device* deviceContext, uint32_t width, uint32_t height, TextureFormat requestFormat, bool mipmap);
    bool init(DX12Device* deviceContext, const ComPtr<ID3D12Resource>& dxRenderTarget);
    void dispose() override;
    DeviceTextureType type() const override { return DeviceTextureType::RenderTarget; }
    SizeI realSize() override { return m_size; }
    TextureFormat getTextureFormat() const { return TextureFormat::RGBA8; }
    GraphicsResourceUsage usage() const override { return GraphicsResourceUsage::Static; }
    void readData(void* outData) override;
    void setSubData(DX12GraphicsContext* graphicsContext, int x, int y, int width, int height, const void* data, size_t dataSize) override {}
    void setSubData3D(DX12GraphicsContext* graphicsContext, int x, int y, int z, int width, int height, int depth, const void* data, size_t dataSize) override {}

    //virtual const DX12Image* image() const override { return m_image.get(); }

    ID3D12Resource* dxResource() const override { return m_dxRenderTarget.Get(); }
    DXGI_FORMAT dxFormat() const { return m_dxFormat; }
    bool isMultisample() const override { return false; }

protected:
    DX12Device* m_deviceContext;
    SizeI m_size;
    TextureFormat m_format;
    
    ComPtr<ID3D12Resource> m_dxRenderTarget;
    DXGI_FORMAT m_dxFormat;
};

class DX12DepthBuffer
    : public IDepthBuffer
{
public:
    DX12DepthBuffer();
    Result init(DX12Device* deviceContext, uint32_t width, uint32_t height);
    void dispose();
    const SizeI& size() const { return m_size; }

    ID3D12Resource* dxResource() const { return m_dxDepthBuffer.Get(); }
    DXGI_FORMAT dxFormat() const { return m_dxFormat; }
    void resourceBarrior(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState);

    bool isMultisample() const override { return false; }

private:
    DX12Device* m_deviceContext;
    SizeI m_size;
    ComPtr<ID3D12Resource> m_dxDepthBuffer;
    DXGI_FORMAT m_dxFormat;
    D3D12_RESOURCE_STATES m_currentState;
};

} // namespace detail
} // namespace ln
