﻿
#include "Internal.hpp"
#include "DX12Buffers.hpp"
#include "DX12Textures.hpp"

#include <LuminoEngine/Graphics/Bitmap.hpp> // TODO: test

namespace ln {
namespace detail {
    
//==============================================================================
// DX12Texture

DX12Texture::DX12Texture()
    : m_mipLevels(1)
    , m_dxFormat(DXGI_FORMAT_UNKNOWN)
    , m_currentState(D3D12_RESOURCE_STATE_COMMON)
{}

void DX12Texture::resourceBarrior(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES state)
{
    if (m_currentState == state) return;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = dxResource();
    barrier.Transition.StateBefore = m_currentState;
    barrier.Transition.StateAfter = state;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);
    m_currentState = state;
}

//==============================================================================
// DX12Texture2D

DX12Texture2D::DX12Texture2D()
{
}

Result DX12Texture2D::init(DX12Device* device, GraphicsResourceUsage usage, uint32_t width, uint32_t height, TextureFormat requestFormat, bool mipmap, const void* initialData)
{
    m_device = device;
	m_usage = usage;
    m_size.width = width;
    m_size.height = height;
    m_format = requestFormat;
    m_currentState = (initialData) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_GENERIC_READ;
    m_dxFormat = DX12Helper::LNTextureFormatToDXFormat(m_format);
    m_mipLevels = 1;
    if (mipmap) {
        m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    }

    D3D12_HEAP_PROPERTIES props;
    props.Type = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    props.CreationNodeMask = device->creationNodeMask();
    props.VisibleNodeMask = device->visibleNodeMask();

    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = m_size.width;
    desc.Height = m_size.height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = m_mipLevels;
    desc.Format = m_dxFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Device* dxDevice = m_device->device();
    HRESULT hr = dxDevice->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        m_currentState,
        nullptr,
        IID_PPV_ARGS(&m_dxResource));

    UINT64 uploadBufferSize = 0;
    dxDevice->GetCopyableFootprints(&desc, 0, 1, 0, &m_footprint, nullptr, nullptr, &uploadBufferSize);
    
    if (initialData) {
        // Upload Buffer は サイズが足りていれば Format は問わない。
        // また D3D12_HEAP_TYPE_DEFAULT に作成されたテクスチャは width にアライメントがついていることがある。
        // そのため横幅の差を考慮して転送しないと画像がくずれる。
        DX12Buffer uploadBuffer;
        if (!uploadBuffer.init(m_device, uploadBufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ)) {
            return false;
        }
        const int pixelSize = GraphicsHelper::getPixelSize(m_format);
        void* data = uploadBuffer.map();
        RHIBitmap bmp1, bmp2;
        bmp1.initWrap(initialData, pixelSize, width, height);
        bmp2.initWritableWrap(data, pixelSize, m_footprint.Footprint.RowPitch / pixelSize, height);
        bmp2.blit(&bmp1);
        uploadBuffer.unmap();

        D3D12_TEXTURE_COPY_LOCATION dst;
        dst.pResource = m_dxResource.Get();
        dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION src;
        src.pResource = uploadBuffer.dxResource();
        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint = m_footprint;

        ID3D12GraphicsCommandList* commandList = m_device->beginSingleTimeCommandList();
        commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
        resourceBarrior(commandList, D3D12_RESOURCE_STATE_GENERIC_READ);
        m_device->endSingleTimeCommandList(commandList);
    }

    if (m_mipLevels >= 2) {
        if (!generateMips()) {
            return false;
        }
    }

	return true;
}

void DX12Texture2D::dispose()
{
    m_dxResource.Reset();
    DX12Texture::dispose();
}

void DX12Texture2D::setSubData(DX12GraphicsContext* graphicsContext, int x, int y, int width, int height, const void* data, size_t dataSize)
{
    LN_NOTIMPLEMENTED();
}

void DX12Texture2D::resourceBarrior(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState)
{
    if (m_currentState == newState) return;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_dxResource.Get();
    barrier.Transition.StateBefore = m_currentState;
    barrier.Transition.StateAfter = newState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);
    m_currentState = newState;
}

struct DWParam
{
    DWParam(FLOAT f) : Float(f) {}
    DWParam(UINT u) : Uint(u) {}

    void operator= (FLOAT f) { Float = f; }
    void operator= (UINT u) { Uint = u; }

    union
    {
        FLOAT Float;
        UINT Uint;
    };
};

bool DX12Texture2D::generateMips()
{
#if 1
    HRESULT hr;
    ID3D12Device* dxDevice = m_device->device();
    const uint32_t width = m_size.width;
    const uint32_t height = m_size.height;
    const uint32_t depth = 1;
    const uint32_t mipMaps = m_mipLevels;
    const uint32_t requiredHeapSize = mipMaps;
    ID3D12GraphicsCommandList* commandList = m_device->beginSingleTimeCommandList();

    D3D12_HEAP_PROPERTIES props;
    props.Type = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    props.CreationNodeMask = m_device->creationNodeMask();
    props.VisibleNodeMask = m_device->visibleNodeMask();

    // 一時バッファ。
    // 最初に元テクスチャをここにコピーし、ComputeShader でダウンサンプリング、最後に書き戻す。
    D3D12_RESOURCE_DESC tempTextureDescriptor = {};
    tempTextureDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    tempTextureDescriptor.Alignment = 0;
    tempTextureDescriptor.Width = width;
    tempTextureDescriptor.Height = height;
    tempTextureDescriptor.DepthOrArraySize = depth;
    tempTextureDescriptor.MipLevels = mipMaps;
    tempTextureDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// TODO:
    tempTextureDescriptor.SampleDesc.Count = 1;
    tempTextureDescriptor.SampleDesc.Quality = 0;
    tempTextureDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    tempTextureDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    ComPtr<ID3D12Resource> tmpTextureResource;
    dxDevice->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &tempTextureDescriptor, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&tmpTextureResource));

    // 元テクスチャの内容を、Subresource 含めてすべてのコピーする
    //D3D12_RESOURCE_BARRIER srcTextureBarrier1 = {};
    //srcTextureBarrier1.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    //srcTextureBarrier1.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    //srcTextureBarrier1.Transition.pResource = tmpTextureResource;
    //srcTextureBarrier1.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    //srcTextureBarrier1.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    //srcTextureBarrier1.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    //commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->_resource, , ));
    resourceBarrior(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->CopyResource(tmpTextureResource.Get(), m_dxResource.Get());

    // まずすべての Subresource を、UNORDERED_ACCESS にする
    D3D12_RESOURCE_BARRIER tmpTextureBarrier2 = {};
    tmpTextureBarrier2.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    tmpTextureBarrier2.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    tmpTextureBarrier2.Transition.pResource = tmpTextureResource.Get();
    tmpTextureBarrier2.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    tmpTextureBarrier2.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    tmpTextureBarrier2.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &tmpTextureBarrier2);

    // 0 番目の Subresource を D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE へ
    D3D12_RESOURCE_BARRIER uavToSrvBarrier = {};
    uavToSrvBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    uavToSrvBarrier.Transition.pResource = tmpTextureResource.Get();
    uavToSrvBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    uavToSrvBarrier.Transition.Subresource = 0;
    uavToSrvBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    uavToSrvBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    commandList->ResourceBarrier(1, &uavToSrvBarrier);




    // CreateRootSignature
    ComPtr<ID3D12RootSignature> mipMapRootSignature;
    {
        // 線形補間 SamplerState
        D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
        samplerDesc.MaxAnisotropy = 0;
        samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
        samplerDesc.ShaderRegister = 0;
        samplerDesc.RegisterSpace = 0;
        samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_DESCRIPTOR_RANGE srvCbvRanges[2];
        D3D12_ROOT_PARAMETER rootParameters[3];
        srvCbvRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvCbvRanges[0].NumDescriptors = 1;
        srvCbvRanges[0].BaseShaderRegister = 0;
        srvCbvRanges[0].RegisterSpace = 0;
        srvCbvRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        srvCbvRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        srvCbvRanges[1].NumDescriptors = 1;
        srvCbvRanges[1].BaseShaderRegister = 0;
        srvCbvRanges[1].RegisterSpace = 0;
        srvCbvRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rootParameters[0].Constants.ShaderRegister = 0;
        rootParameters[0].Constants.RegisterSpace = 0;
        rootParameters[0].Constants.Num32BitValues = 3; // float2 + float
        rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
        rootParameters[1].DescriptorTable.pDescriptorRanges = &srvCbvRanges[0];
        rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
        rootParameters[2].DescriptorTable.pDescriptorRanges = &srvCbvRanges[1];

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.NumParameters = _countof(rootParameters);
        rootSignatureDesc.pParameters = rootParameters;
        rootSignatureDesc.NumStaticSamplers = 1;
        rootSignatureDesc.pStaticSamplers = &samplerDesc;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
        if (FAILED(hr)) {
            LN_ERROR("D3D12SerializeRootSignature failed.");
            return false;
        }

        hr = dxDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mipMapRootSignature));
        if (FAILED(hr)) {
            LN_ERROR("CreateRootSignature failed.");
            return false;
        }
    }

    // CreateComputePipelineState
    ComPtr<ID3D12PipelineState> psoMipMaps;
    {
        ID3DBlob* computeShader = m_device->generateMipMapsShader();

        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = mipMapRootSignature.Get();
        psoDesc.CS = { reinterpret_cast<UINT8*>(computeShader->GetBufferPointer()), computeShader->GetBufferSize() };
        
        hr = dxDevice->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&psoMipMaps));
        if (FAILED(hr)) {
            LN_ERROR("CreateComputePipelineState failed.");
            return false;
        }
    }

    // CreateDescriptorHeap
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    UINT descriptorSize;
    {
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = 2 * requiredHeapSize;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        
        hr = dxDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));
        if (FAILED(hr)) {
            LN_ERROR("CreateDescriptorHeap failed.");
            return false;
        }

        descriptorSize = dxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    commandList->SetComputeRootSignature(mipMapRootSignature.Get());
    commandList->SetPipelineState(psoMipMaps.Get());

    ID3D12DescriptorHeap* heaps[] = { descriptorHeap.Get() };
    commandList->SetDescriptorHeaps(1, heaps);

    D3D12_CPU_DESCRIPTOR_HANDLE currentCPUHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE currentGPUHandle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();



    D3D12_SHADER_RESOURCE_VIEW_DESC srcTextureSRVDesc = {};
    srcTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srcTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srcTextureSRVDesc.Format = tempTextureDescriptor.Format;


    D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
    destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    destTextureUAVDesc.Format = tempTextureDescriptor.Format;


    for (uint32_t TopMip = 0; TopMip < mipMaps - 1; TopMip++)
    {
        uint32_t dstWidth = std::max(width >> (TopMip + 1), 1u);
        uint32_t dstHeight = std::max(height >> (TopMip + 1), 1u);

        // SrcTexture (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
        srcTextureSRVDesc.Texture2D.MipLevels = 1;
        srcTextureSRVDesc.Texture2D.MostDetailedMip = TopMip;
        dxDevice->CreateShaderResourceView(tmpTextureResource.Get(), &srcTextureSRVDesc, currentCPUHandle);
        currentCPUHandle.ptr += descriptorSize;

        // DstTexture (D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        destTextureUAVDesc.Texture2D.MipSlice = TopMip + 1;
        dxDevice->CreateUnorderedAccessView(tmpTextureResource.Get(), nullptr, &destTextureUAVDesc, currentCPUHandle);
        currentCPUHandle.ptr += descriptorSize;

        // ConstantBuffer
        commandList->SetComputeRoot32BitConstant(0, DWParam(1.0f / dstWidth).Uint, 0);  // TexelSize.x
        commandList->SetComputeRoot32BitConstant(0, DWParam(1.0f / dstHeight).Uint, 1); // TexelSize.y
        commandList->SetComputeRoot32BitConstant(0, DWParam(1.0f).Uint, 2);         	// GammaCurve TODO: Gamma

        // DescriptorTable
        commandList->SetComputeRootDescriptorTable(1, currentGPUHandle);
        currentGPUHandle.ptr += descriptorSize;
        commandList->SetComputeRootDescriptorTable(2, currentGPUHandle);
        currentGPUHandle.ptr += descriptorSize;

        // Dispatch
        commandList->Dispatch(std::max(dstWidth / 8, 1u), std::max(dstHeight / 8, 1u), 1);

        // UAV アクセス終了を待つ
        D3D12_RESOURCE_BARRIER tmpTextureBarrire3;
        tmpTextureBarrire3.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        tmpTextureBarrire3.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        tmpTextureBarrire3.UAV.pResource = tmpTextureResource.Get();
        commandList->ResourceBarrier(1, &tmpTextureBarrire3);

        // D3D12_RESOURCE_STATE_UNORDERED_ACCESS -> D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
        uavToSrvBarrier.Transition.Subresource = TopMip + 1;
        commandList->ResourceBarrier(1, &uavToSrvBarrier);
    }

    // 結果を元のテクスチャに戻す
    {
        resourceBarrior(commandList, D3D12_RESOURCE_STATE_COPY_DEST);

        D3D12_RESOURCE_BARRIER tmpTextureBarrier3 = {};
        tmpTextureBarrier3.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        tmpTextureBarrier3.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        tmpTextureBarrier3.Transition.pResource = tmpTextureResource.Get();
        tmpTextureBarrier3.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        tmpTextureBarrier3.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        tmpTextureBarrier3.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &tmpTextureBarrier3);
       
        commandList->CopyResource(m_dxResource.Get(), tmpTextureResource.Get());

        resourceBarrior(commandList, D3D12_RESOURCE_STATE_GENERIC_READ);
    }

    m_device->endSingleTimeCommandList(commandList);
#endif
    return true;
}

//==============================================================================
// DX12RenderTarget

DX12RenderTarget::DX12RenderTarget()
    : m_device(nullptr)
{
}

bool DX12RenderTarget::init(DX12Device* device, uint32_t width, uint32_t height, TextureFormat requestFormat, bool mipmap)
{
    m_device = device;
    m_size.width = width;
    m_size.height = height;
    m_format = requestFormat;
    m_currentState = D3D12_RESOURCE_STATE_GENERIC_READ;
    m_dxFormat = DX12Helper::LNTextureFormatToDXFormat(m_format);
    m_mipLevels = 1;

    D3D12_HEAP_PROPERTIES props;
    props.Type = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    props.CreationNodeMask = device->creationNodeMask();
    props.VisibleNodeMask = device->visibleNodeMask();

    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = m_size.width;
    desc.Height = m_size.height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = m_mipLevels;
    desc.Format = m_dxFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue;
    clearValue.Format = m_dxFormat;
    clearValue.Color[0] = 1.0f;
    clearValue.Color[1] = 1.0f;
    clearValue.Color[2] = 1.0f;
    clearValue.Color[3] = 1.0f;

    ID3D12Device* dxDevice = m_device->device();
    HRESULT hr = dxDevice->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        m_currentState,
        &clearValue,
        IID_PPV_ARGS(&m_dxResource));
    if (FAILED(hr)) {
        LN_ERROR("CreateCommittedResource failed.");
        return false;
    }

    return true;
}

bool DX12RenderTarget::init(DX12Device* device, const ComPtr<ID3D12Resource>& dxRenderTarget)
{
    m_device = device;
    m_dxResource = dxRenderTarget;

    D3D12_RESOURCE_DESC desc = m_dxResource->GetDesc();
    m_size.width = desc.Width;
    m_size.height = desc.Height;
    m_format = DX12Helper::DXFormatToLNTextureFormat(desc.Format);
    m_dxFormat = desc.Format;
    m_currentState = D3D12_RESOURCE_STATE_PRESENT;
    if (LN_REQUIRE(m_format != TextureFormat::Unknown)) return false;

    return true;
}

void DX12RenderTarget::dispose()
{
    m_dxResource.Reset();
    DX12Texture::dispose();
}

RHIPtr<RHIBitmap> DX12RenderTarget::readData()
{
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    D3D12_RESOURCE_DESC textureDesc = m_dxResource->GetDesc();
    UINT64 totalSize;
    m_device->device()->GetCopyableFootprints(&textureDesc, 0, 1, 0, &footprint, nullptr, nullptr, &totalSize);


    // 読み取り用一時バッファ
    size_t size = totalSize;//m_size.width * m_size.height * DX12Helper::getFormatSize(m_dxFormat);
    size_t size2 = m_size.width * m_size.height * DX12Helper::getFormatSize(m_dxFormat);//footprint.Footprint.RowPitch * footprint.Footprint.Height;
    DX12Buffer buffer;
    if (!buffer.init(m_device, size, D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST)) {
        return nullptr;
    }



    ID3D12GraphicsCommandList* commandList = m_device->beginSingleTimeCommandList();
    if (!commandList) {
        return nullptr;
    }

    D3D12_TEXTURE_COPY_LOCATION src, dst;
    src.pResource = m_dxResource.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex = 0;
    dst.pResource = buffer.dxResource();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dst.PlacedFootprint = footprint;

    //D3D12_BOX box;
    //box.left = 0;
    //box.top = 0;
    //box.front = 0;
    //box.right = m_size.width;
    //box.bottom = m_size.height;
    //box.back = 1;
    resourceBarrior(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
    resourceBarrior(commandList, D3D12_RESOURCE_STATE_GENERIC_READ);

    if (!m_device->endSingleTimeCommandList(commandList)) {
        return nullptr;
    }

    const void* data = buffer.map();


    // width が 160 だと 192 に Alignment されることがあった。
    // 余分は切り捨てて返す。
    {
        auto bitmap1 = makeRHIRef<RHIBitmap>();
        if (!bitmap1->init(4, footprint.Footprint.RowPitch / 4, footprint.Footprint.Height)) {
            return nullptr;
        }
        bitmap1->copyRaw(data, totalSize);

        auto bitmap2 = makeRHIRef<RHIBitmap>();
        if (!bitmap2->init(4, m_size.width, m_size.height)) {
            return nullptr;
        }
        bitmap2->blit(bitmap1.get());

        return bitmap2;
    }


    //auto bitmap = makeObject<Bitmap2D>(w, m_size.height, PixelFormat::RGBA8, data);
    //bitmap->save(u"test2.png");



    //memcpy(outData, data, size2);
    //buffer.unmap();
}

//==============================================================================
// DX12DepthBuffer

DX12DepthBuffer::DX12DepthBuffer()
    : m_deviceContext(nullptr)
    , m_size(0, 0)
    , m_dxDepthBuffer()
{
}

Result DX12DepthBuffer::init(DX12Device* deviceContext, uint32_t width, uint32_t height)
{
    LN_DCHECK(deviceContext);
    if (LN_REQUIRE(width > 0)) return false;
    if (LN_REQUIRE(height > 0)) return false;
    m_deviceContext = deviceContext;
    m_size.width = width;
    m_size.height = height;

    m_dxFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    m_currentState = D3D12_RESOURCE_STATE_DEPTH_READ;

    ID3D12Device* dxDevice = m_deviceContext->device();

    D3D12_HEAP_PROPERTIES props;
    props.Type = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    props.CreationNodeMask = m_deviceContext->creationNodeMask();
    props.VisibleNodeMask = m_deviceContext->visibleNodeMask();

    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = m_size.width;
    desc.Height = m_size.height;
    desc.DepthOrArraySize = 1;  // This depth stencil view has only one texture.
    desc.MipLevels = 1;         // Use a single mipmap level.
    desc.Format = m_dxFormat;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = m_dxFormat;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    if (FAILED(dxDevice->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        m_currentState,
        &clearValue,
        IID_PPV_ARGS(&m_dxDepthBuffer)))) {
        LN_ERROR("CreateCommittedResource failed.");
        return false;
    }

    return true;
}

void DX12DepthBuffer::dispose()
{
    m_dxDepthBuffer.Reset();
    IDepthBuffer::dispose();
}

void DX12DepthBuffer::resourceBarrior(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState)
{
    if (m_currentState == newState) return;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_dxDepthBuffer.Get();
    barrier.Transition.StateBefore = m_currentState;
    barrier.Transition.StateAfter = newState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &barrier);
    m_currentState = newState;
}

} // namespace detail
} // namespace ln