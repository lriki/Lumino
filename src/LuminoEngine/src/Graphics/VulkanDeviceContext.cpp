﻿
#include "Internal.hpp"
#include <LuminoEngine/Platform/PlatformWindow.hpp>
#include <LuminoEngine/Platform/PlatformSupport.hpp>
#include "VulkanDeviceContext.hpp"

namespace ln {
namespace detail {

PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback = nullptr;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback = nullptr;
PFN_vkDebugReportMessageEXT vkDebugReportMessage = nullptr;
VkDebugReportCallbackEXT vkDebugReportCallback = 0;

#if defined(VK_EXT_debug_marker)
PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag = nullptr;
PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName = nullptr;
PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin = nullptr;
PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd = nullptr;
PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert = nullptr;
#endif

#if defined(VK_KHR_push_descriptor)
PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSet = nullptr;
#endif

#if defined(VK_EXT_hdr_metadata)
PFN_vkSetHdrMetadataEXT vkSetHdrMetadata = nullptr;
#endif

VKAPI_ATTR
void* VKAPI_CALL AllocCallback(
    void* pUserData,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope scope)
{
    VulkanAllocator* allocator = reinterpret_cast<VulkanAllocator*>(pUserData);
    return allocator->alloc(size, alignment, scope);
}

VKAPI_ATTR
void* VKAPI_CALL ReallocCallback(
    void* pUserData,
    void* pOriginal,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope scope)
{
    VulkanAllocator* allocator = reinterpret_cast<VulkanAllocator*>(pUserData);
    return allocator->realloc(pOriginal, size, alignment, scope);
}

VKAPI_ATTR
void VKAPI_CALL FreeCallback(void* pUserData, void* pMemory)
{
    VulkanAllocator* allocator = reinterpret_cast<VulkanAllocator*>(pUserData);
    return allocator->free(pMemory);
}

VKAPI_ATTR
VkBool32 VKAPI_CALL DebugReportCallback(
    VkFlags msgFlags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t srcObject,
    size_t location,
    int32_t msgCode,
    const char* pLayerPrefix,
    const char* pMsg,
    void* pUserData)
{
    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        LN_LOG_ERROR << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;
    } else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        LN_LOG_WARNING << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;
    } else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        LN_LOG_INFO << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;
    } else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        LN_LOG_DEBUG << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;
    } else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        LN_LOG_DEBUG << "Performance Warning [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;
    }

    return VK_TRUE;
}

struct FormatConversionItem
{
    VkFormat vulkanFormat;
    uint32_t bitPerPixel;
    TextureFormat lnFormat;
    bool isCompress;
};

static FormatConversionItem g_formatConversionTable[] =
    {
        {VK_FORMAT_UNDEFINED, 0, TextureFormat::Unknown, false},
        {VK_FORMAT_R8G8B8A8_UNORM, 32, TextureFormat::RGBA32, false},
        {VK_FORMAT_UNDEFINED, 0, TextureFormat::RGB24, false}, // TODO: remove
        {VK_FORMAT_R16G16B16A16_SFLOAT, 64, TextureFormat::R16G16B16A16Float, false},
        {VK_FORMAT_R32G32B32A32_SFLOAT, 128, TextureFormat::R32G32B32A32Float, false},
        {VK_FORMAT_R16_SFLOAT, 16, TextureFormat::R16Float, false},
        {VK_FORMAT_R32_SFLOAT, 32, TextureFormat::R32Float, false},
        {VK_FORMAT_R32_UINT, 32, TextureFormat::R32UInt, false},
};

static VkFormat LNFormatToVkFormat(TextureFormat format)
{
    assert(g_formatConversionTable[(int)format].lnFormat == format);
    return g_formatConversionTable[(int)format].vulkanFormat;
}

struct BlendFactorConversionItem
{
    BlendFactor lnValue;
    VkBlendFactor vkValueColor;
    VkBlendFactor vkValueAlpha;
};

static const BlendFactorConversionItem s_blendFactorConversionTable[] =
    {
        {BlendFactor::Zero, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO},
        {BlendFactor::One, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE},
        {BlendFactor::SourceColor, VK_BLEND_FACTOR_SRC_COLOR, VK_BLEND_FACTOR_SRC_ALPHA},
        {BlendFactor::InverseSourceColor, VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
        {BlendFactor::SourceAlpha, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_SRC_ALPHA},
        {BlendFactor::InverseSourceAlpha, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
        {BlendFactor::DestinationColor, VK_BLEND_FACTOR_DST_COLOR, VK_BLEND_FACTOR_DST_ALPHA},
        {BlendFactor::InverseDestinationColor, VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA},
        {BlendFactor::DestinationAlpha, VK_BLEND_FACTOR_DST_ALPHA, VK_BLEND_FACTOR_DST_ALPHA},
        {BlendFactor::InverseDestinationAlpha, VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA},
};

static VkBlendFactor LNBlendFactorToVkBlendFactor_Color(BlendFactor value)
{
    assert(s_blendFactorConversionTable[(int)value].lnValue == value);
    return s_blendFactorConversionTable[(int)value].vkValueColor;
}

static VkBlendFactor LNBlendFactorToVkBlendFactor_Alpha(BlendFactor value)
{
    assert(s_blendFactorConversionTable[(int)value].lnValue == value);
    return s_blendFactorConversionTable[(int)value].vkValueAlpha;
}

struct BlendOpConversionItem
{
    BlendOp lnValue;
    VkBlendOp vkValue;
};

static const BlendOpConversionItem s_blendOpConversionTable[] =
    {
        {BlendOp::Add, VK_BLEND_OP_ADD},
        {BlendOp::Subtract, VK_BLEND_OP_SUBTRACT},
        {BlendOp::ReverseSubtract, VK_BLEND_OP_REVERSE_SUBTRACT},
        {BlendOp::Min, VK_BLEND_OP_MIN},
        {BlendOp::Max, VK_BLEND_OP_MAX},
};

static VkBlendOp LNBlendOpToVkBlendOp(BlendOp value)
{
    assert(s_blendOpConversionTable[(int)value].lnValue == value);
    return s_blendOpConversionTable[(int)value].vkValue;
}

struct ComparisonFuncConversionItem
{
    ComparisonFunc lnValue;
    VkCompareOp vkValue;
};

static const ComparisonFuncConversionItem s_comparisoFuncConversionTable[] =
    {
        {ComparisonFunc::Never, VK_COMPARE_OP_NEVER},
        {ComparisonFunc::Less, VK_COMPARE_OP_LESS},
        {ComparisonFunc::LessEqual, VK_COMPARE_OP_LESS_OR_EQUAL},
        {ComparisonFunc::Greater, VK_COMPARE_OP_GREATER},
        {ComparisonFunc::GreaterEqual, VK_COMPARE_OP_GREATER_OR_EQUAL},
        {ComparisonFunc::Equal, VK_COMPARE_OP_EQUAL},
        {ComparisonFunc::NotEqual, VK_COMPARE_OP_NOT_EQUAL},
        {ComparisonFunc::Always, VK_COMPARE_OP_ALWAYS},
};

static VkCompareOp LNComparisonFuncToVkCompareOp(ComparisonFunc value)
{
    assert(s_comparisoFuncConversionTable[(int)value].lnValue == value);
    return s_comparisoFuncConversionTable[(int)value].vkValue;
}

struct FillModeConversionItem
{
    FillMode lnValue;
    VkPolygonMode vkValue;
};

static const FillModeConversionItem s_fillModeConversionTable[] =
    {
        {FillMode::Solid, VK_POLYGON_MODE_FILL},
        {FillMode::Wireframe, VK_POLYGON_MODE_LINE},
};

static VkPolygonMode LNFillModeToVkPolygonMode(FillMode value)
{
    assert(s_fillModeConversionTable[(int)value].lnValue == value);
    return s_fillModeConversionTable[(int)value].vkValue;
}

struct CullModeConversionItem
{
    CullMode lnValue;
    VkCullModeFlagBits vkValue;
};

static const CullModeConversionItem s_cullModeConversionTable[] =
    {
        {CullMode::None, VK_CULL_MODE_NONE},
        {CullMode::Front, VK_CULL_MODE_FRONT_BIT},
        {CullMode::Back, VK_CULL_MODE_BACK_BIT},
};

static VkCullModeFlagBits LNCullModeToVkCullMode(CullMode value)
{
    assert(s_cullModeConversionTable[(int)value].lnValue == value);
    return s_cullModeConversionTable[(int)value].vkValue;
}

struct StencilOpConversionItem
{
    StencilOp lnValue;
    VkStencilOp vkValue;
};

static const StencilOpConversionItem s_stencilOpConversionTable[] =
    {
        {StencilOp::Keep, VK_STENCIL_OP_KEEP},
        {StencilOp::Replace, VK_STENCIL_OP_REPLACE},
};

static VkStencilOp LNStencilOpToVkStencilOp(StencilOp value)
{
    assert(s_stencilOpConversionTable[(int)value].lnValue == value);
    return s_stencilOpConversionTable[(int)value].vkValue;
}

struct VertexElementTypeConversionItem
{
    VertexElementType lnValue;
    VkFormat vkValue;
};

static const VertexElementTypeConversionItem s_vertexElementTypeConversionTable[] =
    {
        {VertexElementType::Unknown, VK_FORMAT_UNDEFINED},

        {VertexElementType::Float1, VK_FORMAT_R32_SFLOAT},
        {VertexElementType::Float2, VK_FORMAT_R32G32_SFLOAT},
        {VertexElementType::Float3, VK_FORMAT_R32G32B32_SFLOAT},
        {VertexElementType::Float4, VK_FORMAT_R32G32B32A32_SFLOAT},

        {VertexElementType::Ubyte4, VK_FORMAT_R8G8B8A8_UINT},
        {VertexElementType::Color4, VK_FORMAT_R8G8B8A8_UNORM}, // UNORM : https://msdn.microsoft.com/ja-jp/library/ee415736%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396

        {VertexElementType::Short2, VK_FORMAT_R16G16_SINT},
        {VertexElementType::Short4, VK_FORMAT_R16G16B16A16_SINT},
};

static VkFormat LNVertexElementTypeToVkFormat(VertexElementType value)
{
    assert(s_vertexElementTypeConversionTable[(int)value].lnValue == value);
    return s_vertexElementTypeConversionTable[(int)value].vkValue;
}

//=============================================================================
// VulkanAllocator

VulkanAllocator::VulkanAllocator()
    : m_counter(0)
{
}

void* VulkanAllocator::alloc(size_t size, size_t alignment, VkSystemAllocationScope scope) noexcept
{
    m_counter++;
    m_allocationSize[scope] -= size;
#ifdef LN_OS_WIN32
    return _aligned_malloc(size, alignment);
#else
    return aligned_alloc(alignment, size);
#endif
}

void* VulkanAllocator::realloc(void* ptr, size_t size, size_t alignment, VkSystemAllocationScope scope) noexcept
{
    m_counter++;
#ifdef LN_OS_WIN32
    return _aligned_realloc(ptr, size, alignment);
#else
    A3D_UNUSED(alignment);
    return realloc(ptr, size);
#endif
}

void VulkanAllocator::free(void* ptr) noexcept
{
    m_counter--;
#ifdef LN_OS_WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}


//=============================================================================
// VulkanPipelineCache

uint64_t VulkanPipelineCache::computeHash(const IGraphicsDeviceContext::State& state)
{
	MixHash hash;
	hash.add(state.blendState);
	hash.add(state.rasterizerState);
	hash.add(state.depthStencilState);
	hash.add(state.vertexDeclaration);
	hash.add(state.shaderPass);
	for (auto& t : state.renderTargets) {
		hash.add(t->getTextureFormat());
	}
	hash.add(state.depthBuffer->format());
	return hash.value();
}


//=============================================================================
// VulkanRenderPassCache

uint64_t VulkanRenderPassCache::computeHash(ITexture* const* renderTargets, size_t renderTargetCount, IDepthBuffer* depthBuffer)
{
	MixHash hash;
	for (size_t i = 0; i < renderTargetCount; i++) {
		hash.add(renderTargets[i]->getTextureFormat());
	}
	hash.add(VK_FORMAT_D32_SFLOAT_S8_UINT);	// TODO: DepthFormat
	return hash.value();
}

//=============================================================================
// VulkanRenderPassCache

bool VulkanFrameBuffer::init(VulkanDeviceContext* deviceContext, ITexture* const* renderTargets, size_t renderTargetCount, IDepthBuffer* depthBuffer)
{
	m_deviceContext = deviceContext;
	m_renderTargetCount = renderTargetCount;
	for (size_t i = 0; i < renderTargetCount; i++) {
		m_renderTargets[i] = renderTargets[i];
	}
	m_depthBuffer = depthBuffer;



	VulkanTextureBase::TextureDesc desc = static_cast<VulkanTextureBase*>(m_renderTargets[0])->desc();


	VkRenderPass renderPass;
	if (!deviceContext->getVkRenderPass(renderTargets, renderTargetCount, depthBuffer, &renderPass)) {
		return false;
	}
	else
	{
		VkImageView attachments[IGraphicsDeviceContext::MaxRenderTargets] = {};
		for (size_t i = 0; i < renderTargetCount; i++) {
			attachments[i] = static_cast<VulkanTextureBase*>(m_renderTargets[i])->vulkanImageView();
		}

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = desc.Width;
		framebufferInfo.height = desc.Height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_deviceContext->vulkanDevice(), &framebufferInfo, m_deviceContext->vulkanAllocator(), &m_framebuffer) != VK_SUCCESS) {
			LN_LOG_ERROR << "Failed vkCreateFramebuffer";
			return false;
		}
	}

	return true;
}

void VulkanFrameBuffer::dispose()
{
	if (m_framebuffer) {
		vkDestroyFramebuffer(m_deviceContext->vulkanDevice(), m_framebuffer, m_deviceContext->vulkanAllocator());
		m_framebuffer = 0;
	}
}

bool VulkanFrameBuffer::containsRenderTarget(ITexture* renderTarget) const
{
	for (size_t i = 0; i < m_renderTargetCount; i++) {
		if (renderTarget == m_renderTargets[i]) {
			return true;
		}
	}
	return false;
}

bool VulkanFrameBuffer::containsDepthBuffer(IDepthBuffer* depthBuffer) const
{
	return m_depthBuffer == depthBuffer;
}

//=============================================================================
// VulkanDeviceContext

VulkanDeviceContext::VulkanDeviceContext()
    : m_instance(nullptr)
    , m_allocatorCallbacks()
    , m_allocator()
{
}

VulkanDeviceContext::~VulkanDeviceContext()
{
}

bool VulkanDeviceContext::init(const Settings& settings)
{
    //#ifdef LN_OS_WIN32
    //    Win32PlatformInterface::getWin32WindowHandle(settings.mainWindow);
    //#endif

    const char* instanceExtension[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef LN_OS_WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif LN_OS_LINUX
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif LN_OS_ANDROID
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#endif
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };
    size_t instanceExtensionCount = 1;

    const char* layerNames[] = {
        "VK_LAYER_LUNARG_standard_validation",
    };
    size_t layerCount = 0;

    if (settings.debugEnabled) {
        instanceExtensionCount++;
        layerCount++;
    }

    std::vector<std::string> extensions;
    CheckInstanceExtension(
        nullptr,
        instanceExtensionCount,
        instanceExtension,
        &extensions);
    std::vector<const char*> extensionPtrs;
    for (auto& str : extensions)
        extensionPtrs.push_back(str.c_str());

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Lumino";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Lumino";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = layerCount;
    instanceInfo.ppEnabledLayerNames = (layerCount == 0) ? nullptr : layerNames;
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensionPtrs.data();

    m_allocatorCallbacks.pfnAllocation = AllocCallback;
    m_allocatorCallbacks.pfnFree = FreeCallback;
    m_allocatorCallbacks.pfnReallocation = ReallocCallback;
    m_allocatorCallbacks.pfnInternalAllocation = nullptr;
    m_allocatorCallbacks.pfnInternalFree = nullptr;
    m_allocatorCallbacks.pUserData = &m_allocator;

    if (vkCreateInstance(&instanceInfo, &m_allocatorCallbacks, &m_instance) != VK_SUCCESS) {
        LN_LOG_ERROR << "Failed vkCreateInstance";
        return false;
    }

    if (settings.debugEnabled) {
        vkCreateDebugReportCallback = GetVkInstanceProc<PFN_vkCreateDebugReportCallbackEXT>("vkCreateDebugReportCallbackEXT");
        vkDestroyDebugReportCallback = GetVkInstanceProc<PFN_vkDestroyDebugReportCallbackEXT>("vkDestroyDebugReportCallbackEXT");
        vkDebugReportMessage = GetVkInstanceProc<PFN_vkDebugReportMessageEXT>("vkDebugReportMessageEXT");

        if (vkCreateDebugReportCallback != nullptr &&
            vkDestroyDebugReportCallback != nullptr &&
            vkDebugReportMessage != nullptr) {
            VkFlags flags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
            flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT;
            flags |= VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
            flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;

            VkDebugReportCallbackCreateInfoEXT info = {};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            info.pNext = nullptr;
            info.pfnCallback = DebugReportCallback;
            info.pUserData = nullptr;
            info.flags = flags;

            if (vkCreateDebugReportCallback(
                    m_instance,
                    &info,
                    nullptr,
                    &vkDebugReportCallback) != VK_SUCCESS) {
                LN_LOG_ERROR << "Failed vkCreateDebugReportCallback";
                return false;
            }
        }
    }

    // Get physical devices
    {
        uint32_t count = 0;
        auto ret = vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
        if (ret != VK_SUCCESS || count < 1) {
            LN_LOG_ERROR << "Failed vkEnumeratePhysicalDevices";
            return false;
        }

        m_physicalDeviceCount = count;
        m_physicalDeviceInfos.resize(m_physicalDeviceCount);

        std::vector<VkPhysicalDevice> gpuDevices(m_physicalDeviceCount);
        if (vkEnumeratePhysicalDevices(m_instance, &count, gpuDevices.data()) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkEnumeratePhysicalDevices";
            return false;
        }

        for (auto i = 0u; i < count; ++i) {
            m_physicalDeviceInfos[i].device = gpuDevices[i];
            vkGetPhysicalDeviceMemoryProperties(gpuDevices[i], &m_physicalDeviceInfos[i].memoryProperty);
            vkGetPhysicalDeviceProperties(gpuDevices[i], &m_physicalDeviceInfos[i].deviceProperty);
        }
    }

    // Select device
    VkPhysicalDevice physicalDevice = m_physicalDeviceInfos[0].device;

    // Create Device and Queue
    {
        auto graphicsFamilyIndex = UINT32_MAX;
        auto computeFamilyIndex = UINT32_MAX;
        auto transferFamilyIndex = UINT32_MAX;

        auto graphicsQueueIndex = UINT32_MAX;
        auto computeQueueIndex = UINT32_MAX;
        auto transferQueueindex = UINT32_MAX;

        {
            uint32_t propCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilyPros(propCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propCount, queueFamilyPros.data());

            std::vector<VkDeviceQueueCreateInfo> queueInfos(propCount);
            int queueIndex = 0;
            int totalQueueCount = 0;
            for (int i = 0; i < propCount; ++i) {
                queueInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueInfos[i].pNext = nullptr;
                queueInfos[i].flags = 0;
                queueInfos[i].queueCount = queueFamilyPros[i].queueCount;
                queueInfos[i].queueFamilyIndex = i;

                totalQueueCount += queueFamilyPros[i].queueCount;

                // Graphics queue
                if (queueFamilyPros[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    if (graphicsFamilyIndex == UINT32_MAX) {
                        graphicsFamilyIndex = i;
                        graphicsQueueIndex = queueIndex;
                        queueIndex++;
                    }
                }

                // Compute queue
                if ((queueFamilyPros[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyPros[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT)) {
                    if (computeFamilyIndex == UINT32_MAX) {
                        computeFamilyIndex = i;
                        computeQueueIndex = queueIndex;
                        queueIndex++;
                    }
                }

                // Transfer queue
                if ((queueFamilyPros[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilyPros[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != VK_QUEUE_GRAPHICS_BIT)) {
                    if (transferFamilyIndex == UINT32_MAX) {
                        transferFamilyIndex = i;
                        transferQueueindex = queueIndex;
                        queueIndex++;
                    }
                }
            }

            // 1つも見つからなければ仕方ないので共用のものを探す.
            if (computeFamilyIndex == UINT32_MAX) {
                for (auto i = 0u; i < propCount; ++i) {
                    if (queueFamilyPros[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                        if (computeFamilyIndex == UINT32_MAX) {
                            computeFamilyIndex = i;
                            computeQueueIndex = queueIndex;
                            queueIndex++;
                        }
                    }
                }
            }

            // 1つも見つからなければ仕方ないので共用のものを探す.
            if (transferFamilyIndex == UINT32_MAX) {
                for (auto i = 0u; i < propCount; ++i) {
                    if (queueFamilyPros[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
                        if (transferFamilyIndex == UINT32_MAX) {
                            transferFamilyIndex = i;
                            transferQueueindex = queueIndex;
                            queueIndex++;
                        }
                    }
                }
            }

            //auto pPriorities = new float[totalQueueCount];
            //if (pPriorities == nullptr)
            //{
            //    delete[] pProps;
            //    delete[] pQueueInfos;
            //    return false;
            //}

            //memset(pPriorities, 0, sizeof(float) * totalQueueCount);

            std::vector<float> queuePriorities(totalQueueCount);
            uint32_t offset = 0u;
            for (uint32_t i = 0u; i < propCount; ++i) {
                queueInfos[i].pQueuePriorities = &queuePriorities[offset];
                offset += queueInfos[i].queueCount;
            }

            //a3d::dynamic_array<char*> deviceExtensions;
            std::vector<std::string> deviceExtensions;
            if (settings.debugEnabled) {
                deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            } else {
                GetDeviceExtension(
                    nullptr,
                    physicalDevice,
                    &deviceExtensions);

                m_ext_EXT_KHR_PUSH_DESCRIPTOR = false;
                m_ext_EXT_KHR_DESCRIPTOR_UPDATE_TEMPLATE = false;
                m_ext_EXT_NVX_DEVICE_GENERATE_COMMAND = false;
                for (size_t i = 0; i < deviceExtensions.size(); ++i) {
#if defined(VK_KHR_push_descriptor)
                    if (deviceExtensions[i] == VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) {
                        m_ext_EXT_KHR_PUSH_DESCRIPTOR = true;
                    }
                    if (deviceExtensions[i] == VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME) {
                        m_ext_EXT_KHR_DESCRIPTOR_UPDATE_TEMPLATE = true;
                    }
#endif
#if defined(VK_NVX_device_generated_commands)
                    if (deviceExtensions[i] == VK_NVX_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME) {
                        m_ext_EXT_NVX_DEVICE_GENERATE_COMMAND = true;
                    }
#endif
#if defined(VK_AMD_draw_indirect_count)
                    if (deviceExtensions[i] == VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME) {
                        m_ext_EXT_AMD_DRAW_INDIRECT_COUNT = true;
                    }
#endif
#if defined(VK_EXT_debug_marker)
                    if (deviceExtensions[i] == VK_EXT_DEBUG_MARKER_EXTENSION_NAME) {
                        m_ext_EXT_DEBUG_MARKER = true;
                    }
#endif
#if defined(VK_EXT_hdr_metadata)
                    if (deviceExtensions[i] == VK_EXT_HDR_METADATA_EXTENSION_NAME) {
                        m_ext_EXT_HDR_METADATA = true;
                    }
#endif
                }
            }

            std::vector<const char*> deviceExtensionPtrs;
            for (auto& str : deviceExtensions)
                deviceExtensionPtrs.push_back(str.c_str());

            VkDeviceCreateInfo deviceInfo = {};
            deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceInfo.pNext = nullptr;
            deviceInfo.queueCreateInfoCount = propCount;
            deviceInfo.pQueueCreateInfos = queueInfos.data();
            deviceInfo.enabledLayerCount = 0;         // deprecated https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkDeviceCreateInfo.html
            deviceInfo.ppEnabledLayerNames = nullptr; // deprecated https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkDeviceCreateInfo.html
            deviceInfo.enabledExtensionCount = (uint32_t)deviceExtensionPtrs.size();
            deviceInfo.ppEnabledExtensionNames = deviceExtensionPtrs.data();
            deviceInfo.pEnabledFeatures = nullptr;

            if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &m_device) != VK_SUCCESS) {
                LN_LOG_ERROR << "Failed vkCreateDevice";
                return false;
            }
        }

#if defined(VK_EXT_debug_marker)
        if (m_ext_EXT_DEBUG_MARKER) {
            vkDebugMarkerSetObjectTag = GetVkDeviceProc<PFN_vkDebugMarkerSetObjectTagEXT>("vkDebugMarkerSetObjectTagEXT");
            vkDebugMarkerSetObjectName = GetVkDeviceProc<PFN_vkDebugMarkerSetObjectNameEXT>("vkDebugMarkerSetObjectNameEXT");
            vkCmdDebugMarkerBegin = GetVkDeviceProc<PFN_vkCmdDebugMarkerBeginEXT>("vkCmdDebugMarkerBeginEXT");
            vkCmdDebugMarkerEnd = GetVkDeviceProc<PFN_vkCmdDebugMarkerEndEXT>("vkCmdDebugMarkerEndEXT");
            vkCmdDebugMarkerInsert = GetVkDeviceProc<PFN_vkCmdDebugMarkerInsertEXT>("vkCmdDebugMarkerInsert");
        }
#endif
#if defined(VK_KHR_push_descriptor)
        if (m_ext_EXT_KHR_PUSH_DESCRIPTOR) {
            vkCmdPushDescriptorSet = GetVkDeviceProc<PFN_vkCmdPushDescriptorSetKHR>("vkCmdPushDescriptorSetKHR");
        }
#endif
#if defined(VK_EXT_hdr_metadata)
        if (m_ext_EXT_HDR_METADATA) {
            vkSetHdrMetadata = GetVkDeviceProc<PFN_vkSetHdrMetadataEXT>("vkSetHdrMetadataEXT");
        }
#endif
        m_graphicsQueue = makeRef<VulkanQueue>();
        if (!m_graphicsQueue->init(this, graphicsFamilyIndex, graphicsQueueIndex, settings.maxGraphicsQueueSubmitCount)) {
            return false;
        }

        m_computeQueue = makeRef<VulkanQueue>();
        if (!m_computeQueue->init(this, computeFamilyIndex, computeQueueIndex, settings.maxComputeQueueSubmitCount)) {
            return false;
        }

        m_transferQueue = makeRef<VulkanQueue>();
        if (!m_transferQueue->init(this, transferFamilyIndex, transferQueueindex, settings.maxTransferQueueSubmitCount)) {
            return false;
        }
    }

    // Get device infomation
    {
        auto& limits = m_physicalDeviceInfos[0].deviceProperty.limits;
        m_caps.ConstantBufferMemoryAlignment = static_cast<uint32_t>(limits.minUniformBufferOffsetAlignment);
        m_caps.MaxTargetWidth = limits.maxFramebufferWidth;
        m_caps.MaxTargetHeight = limits.maxFramebufferHeight;
        m_caps.MaxTargetArraySize = limits.maxFramebufferLayers;
        m_caps.MaxColorSampleCount = static_cast<uint32_t>(limits.framebufferColorSampleCounts);
        m_caps.MaxDepthSampleCount = static_cast<uint32_t>(limits.framebufferDepthSampleCounts);
        m_caps.MaxStencilSampleCount = static_cast<uint32_t>(limits.framebufferStencilSampleCounts);

        if (limits.timestampComputeAndGraphics) {
            auto nanoToSec = 1000 * 1000 * 1000;
            m_timeStampFrequency = static_cast<uint64_t>(limits.timestampPeriod * nanoToSec);
        } else {
            m_timeStampFrequency = 1;
        }
    }

	if (!m_renderPassCache.init([this](VkRenderPass v) { vkDestroyRenderPass(m_device, v, &m_allocatorCallbacks); })) {
        return false;
    }

    if (!m_pipelineCache.init([this](Ref<VulkanPipeline> v) { v->dispose(); })) {
        return false;
    }

	if (!m_frameBufferCache.init([this](Ref<VulkanFrameBuffer> v) { v->dispose(); })) {
		return false;
	}

    return true;
}

void VulkanDeviceContext::dispose()
{
	m_frameBufferCache.clear();
    m_pipelineCache.clear();
    m_renderPassCache.clear();

    if (m_transferQueue) {
        m_transferQueue->dispose();
        m_transferQueue = nullptr;
    }
    if (m_computeQueue) {
        m_computeQueue->dispose();
        m_computeQueue = nullptr;
    }
    if (m_graphicsQueue) {
        m_graphicsQueue->dispose();
        m_graphicsQueue = nullptr;
    }

    if (m_device) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device = nullptr;
    }

    if (vkDebugReportCallback != 0) {
        vkDestroyDebugReportCallback(
            m_instance,
            vkDebugReportCallback,
            nullptr);
        vkDebugReportCallback = 0;
    }

    if (m_instance) {
        vkDestroyInstance(m_instance, &m_allocatorCallbacks);
        m_instance = nullptr;
    }

    IGraphicsDeviceContext::dispose();
}

bool VulkanDeviceContext::getVkRenderPass(ITexture* const* renderTargets, size_t renderTargetCount, IDepthBuffer* depthBuffer, VkRenderPass* outPass)
{
	uint64_t hash = VulkanRenderPassCache::computeHash(renderTargets, renderTargetCount, depthBuffer);
	VkRenderPass renderPass = 0;
	if (renderPassCache().find(hash, &renderPass)) {
		// use renderPass
	}
	else
	{
		// MaxRenderTargets + 1枚の depthbuffer
		VkAttachmentDescription attachmentDescs[IGraphicsDeviceContext::MaxRenderTargets + 1] = {};
		VkAttachmentReference attachmentRefs[IGraphicsDeviceContext::MaxRenderTargets + 1] = {};
		VkAttachmentReference* depthAttachmentRef = nullptr;
		int attachmentCount = 0;
		int colorAttachmentCount = 0;

		for (int i = 0; i < IGraphicsDeviceContext::MaxRenderTargets; i++) {
			if (renderTargets[i]) {
				attachmentDescs[i].flags = 0;
				attachmentDescs[i].format = LNFormatToVkFormat(renderTargets[i]->getTextureFormat());
				attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				attachmentRefs[i].attachment = attachmentCount;
				attachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				attachmentCount++;
				colorAttachmentCount++;
			}
			else {
				break;
			}
		}

		if (depthBuffer) {
			int i = colorAttachmentCount;

			attachmentDescs[i].flags = 0;
			attachmentDescs[i].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
			attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			attachmentRefs[i].attachment = attachmentCount;
			attachmentRefs[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			depthAttachmentRef = &attachmentRefs[i];
			attachmentCount++;
		}

		VkSubpassDescription subpass;
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.colorAttachmentCount = colorAttachmentCount;
		subpass.pColorAttachments = attachmentRefs;
		subpass.pResolveAttachments = nullptr;
		subpass.pDepthStencilAttachment = depthAttachmentRef;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.flags = 0;
		renderPassInfo.attachmentCount = attachmentCount;
		renderPassInfo.pAttachments = attachmentDescs;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 0;
		renderPassInfo.pDependencies = nullptr;

		if (vkCreateRenderPass(vulkanDevice(), &renderPassInfo, vulkanAllocator(), &renderPass) != VK_SUCCESS) {
			LN_LOG_ERROR << "Failed vkCreateRenderPass";
			return false;
		}

		renderPassCache().add(hash, renderPass);
	}

	*outPass = renderPass;
	return true;
}

VkPhysicalDevice VulkanDeviceContext::vulkanPhysicalDevice() const
{
    return m_physicalDeviceInfos[0].device;
}

void VulkanDeviceContext::onGetCaps(GraphicsDeviceCaps* outCaps)
{
    outCaps->requestedShaderTriple.target = "spv";
    outCaps->requestedShaderTriple.version = 0;
    outCaps->requestedShaderTriple.option = "";
}

void VulkanDeviceContext::onEnterMainThread()
{
}

void VulkanDeviceContext::onLeaveMainThread()
{
}

void VulkanDeviceContext::onSaveExternalRenderState()
{
}

void VulkanDeviceContext::onRestoreExternalRenderState()
{
}

Ref<ISwapChain> VulkanDeviceContext::onCreateSwapChain(PlatformWindow* window, const SizeI& backbufferSize)
{
    VulkanSwapChain::SwapChainDesc desc;
    desc.Width = backbufferSize.width;
    desc.Height = backbufferSize.height;
    desc.Format = TextureFormat::RGBA32;
    desc.MipLevels = 1;
    desc.SampleCount = 1;
    desc.BufferCount = 2;
    desc.SyncInterval = 1;
    desc.EnableFullScreen = false;

    auto ptr = makeRef<VulkanSwapChain>();
    if (!ptr->init(this, window, desc)) {
        return nullptr;
    }

    return ptr;
}

Ref<IVertexDeclaration> VulkanDeviceContext::onCreateVertexDeclaration(const VertexElement* elements, int elementsCount)
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

Ref<IVertexBuffer> VulkanDeviceContext::onCreateVertexBuffer(GraphicsResourceUsage usage, size_t bufferSize, const void* initialData)
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

Ref<IIndexBuffer> VulkanDeviceContext::onCreateIndexBuffer(GraphicsResourceUsage usage, IndexBufferFormat format, int indexCount, const void* initialData)
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

Ref<ITexture> VulkanDeviceContext::onCreateTexture2D(uint32_t width, uint32_t height, TextureFormat requestFormat, bool mipmap, const void* initialData)
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

Ref<ITexture> VulkanDeviceContext::onCreateTexture3D(uint32_t width, uint32_t height, uint32_t depth, TextureFormat requestFormat, bool mipmap, const void* initialData)
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

Ref<ITexture> VulkanDeviceContext::onCreateRenderTarget(uint32_t width, uint32_t height, TextureFormat requestFormat, bool mipmap)
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

Ref<IDepthBuffer> VulkanDeviceContext::onCreateDepthBuffer(uint32_t width, uint32_t height)
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

Ref<ISamplerState> VulkanDeviceContext::onCreateSamplerState(const SamplerStateData& desc)
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

Ref<IShaderPass> VulkanDeviceContext::onCreateShaderPass(const byte_t* vsCode, int vsCodeLen, const byte_t* psCode, int psCodeLen, ShaderCompilationDiag* diag)
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

void VulkanDeviceContext::onUpdatePipelineState(const BlendStateDesc& blendState, const RasterizerStateDesc& rasterizerState, const DepthStencilStateDesc& depthStencilState)
{
    LN_NOTIMPLEMENTED();
}

void VulkanDeviceContext::onUpdateFrameBuffers(ITexture** renderTargets, int renderTargetsCount, IDepthBuffer* depthBuffer)
{
    LN_NOTIMPLEMENTED();
}

void VulkanDeviceContext::onUpdateRegionRects(const RectI& viewportRect, const RectI& scissorRect, const SizeI& targetSize)
{
    LN_NOTIMPLEMENTED();
}

void VulkanDeviceContext::onUpdatePrimitiveData(IVertexDeclaration* decls, IVertexBuffer** vertexBuufers, int vertexBuffersCount, IIndexBuffer* indexBuffer)
{
    LN_NOTIMPLEMENTED();
}

void VulkanDeviceContext::onUpdateShaderPass(IShaderPass* newPass)
{
    LN_NOTIMPLEMENTED();
}

void VulkanDeviceContext::onClearBuffers(ClearFlags flags, const Color& color, float z, uint8_t stencil)
{
    const State& committed = committedState();



	{
	}

    LN_NOTIMPLEMENTED();
}

void VulkanDeviceContext::onDrawPrimitive(PrimitiveType primitive, int startVertex, int primitiveCount)
{
    LN_NOTIMPLEMENTED();
}

void VulkanDeviceContext::onDrawPrimitiveIndexed(PrimitiveType primitive, int startIndex, int primitiveCount)
{
    LN_NOTIMPLEMENTED();
}

void VulkanDeviceContext::onPresent(ISwapChain* swapChain)
{
    LN_NOTIMPLEMENTED();
}

// 要求したインスタンスの拡張が本当に使えるか確認する
void VulkanDeviceContext::CheckInstanceExtension(
    const char* layer,
    size_t requestCount,
    const char** requestNames,
    std::vector<std::string>* result)
{
    uint32_t count;
    auto t = vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);

    std::vector<VkExtensionProperties> exts;
    exts.resize(count);
    vkEnumerateInstanceExtensionProperties(layer, &count, exts.data());

    result->reserve(count);
    for (size_t i = 0; i < exts.size(); ++i) {
        bool hit = false;
        for (size_t j = 0; j < requestCount; ++j) {
            if (strcmp(exts[i].extensionName, requestNames[j]) == 0) {
                hit = true;
                break;
            }
        }

        if (!hit) {
            continue;
        }

        result->push_back(exts[i].extensionName);
    }
}

void VulkanDeviceContext::GetDeviceExtension(
    const char* layer,
    VkPhysicalDevice physicalDevice,
    std::vector<std::string>* result)
{
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, layer, &count, nullptr);

    std::vector<VkExtensionProperties> exts;
    exts.resize(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, layer, &count, exts.data());

    result->reserve(count);
    for (size_t i = 0; i < exts.size(); ++i) {
        result->push_back(exts[i].extensionName);
    }
}

//=============================================================================
// VulkanQueue

VulkanQueue::VulkanQueue()
    : m_deviceContext(nullptr)
    , m_familyIndex(0)
    , m_maxSubmitCount(0)
    , m_queue(nullptr)
    , m_signalSemaphore{}
    , m_waitSemaphore{}
    , m_fence{}
{
    for (uint32_t i = 0; i < MaxBufferCount; i++) {
        m_waitSemaphore[i] = 0;
        m_signalSemaphore[i] = 0;
        m_fence[i] = 0;
    }
}

bool VulkanQueue::init(VulkanDeviceContext* deviceContext, uint32_t familyIndex, uint32_t queueIndex, uint32_t maxSubmitCount)
{
    if (LN_REQUIRE(deviceContext)) return false;
    m_deviceContext = deviceContext;
    VkDevice vulkanDevice = m_deviceContext->vulkanDevice();

    // Create semaphore
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        for (int i = 0; i < MaxBufferCount; i++) {
            if (vkCreateSemaphore(vulkanDevice, &info, nullptr, &m_signalSemaphore[i]) != VK_SUCCESS) {
                LN_LOG_ERROR << "Failed vkCreateSemaphore";
                return false;
            }
            if (vkCreateSemaphore(vulkanDevice, &info, nullptr, &m_waitSemaphore[i]) != VK_SUCCESS) {
                LN_LOG_ERROR << "Failed vkCreateSemaphore";
                return false;
            }
        }
    }

    // Create fence
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        for (auto i = 0; i < MaxBufferCount; ++i) {
            if (vkCreateFence(vulkanDevice, &info, nullptr, &m_fence[i]) != VK_SUCCESS) {
                LN_LOG_ERROR << "Failed vkCreateFence";
                return false;
            }

            if (vkResetFences(vulkanDevice, 1, &m_fence[i]) != VK_SUCCESS) {
                info.flags = 0;
            }
        }
    }

    vkGetDeviceQueue(vulkanDevice, familyIndex, queueIndex, &m_queue);

    m_maxSubmitCount = maxSubmitCount;
    m_familyIndex = familyIndex;

    m_submitList.resize(maxSubmitCount);
    for (int i = 0; i < maxSubmitCount; i++) {
        m_submitList[i] = 0;
    }

    m_submitIndex = 0;
    m_currentBufferIndex = 0;
    m_previousBufferIndex = 0;

    return true;
}

void VulkanQueue::dispose()
{
    VkDevice vulkanDevice = m_deviceContext->vulkanDevice();

    // Wait for complation
    if (m_queue) {
        vkQueueWaitIdle(m_queue);
    }

    for (uint32_t i = 0; i < MaxBufferCount; i++) {
        if (m_signalSemaphore[i]) {
            vkDestroySemaphore(vulkanDevice, m_signalSemaphore[i], nullptr);
            m_signalSemaphore[i] = 0;
        }

        if (m_waitSemaphore[i]) {
            vkDestroySemaphore(vulkanDevice, m_waitSemaphore[i], nullptr);
            m_waitSemaphore[i] = 0;
        }

        if (m_fence[i]) {
            vkDestroyFence(vulkanDevice, m_fence[i], nullptr);
            m_fence[i] = 0;
        }
    }

    m_submitList.clear();
    m_queue = 0;
}

//=============================================================================
// VulkanCommandList

VulkanCommandList::VulkanCommandList()
{
}

VulkanCommandList::~VulkanCommandList()
{
}

bool VulkanCommandList::init(VulkanDeviceContext* deviceContext, Type type)
{
    m_deviceContext = deviceContext;

    // Create command pool
    {
        uint32_t queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        VulkanQueue* queue = nullptr;
        if (type == Type::COMMANDLIST_TYPE_DIRECT) {
            queue = m_deviceContext->graphicsQueue();
        } else if (type == Type::COMMANDLIST_TYPE_COMPUTE) {
            queue = m_deviceContext->computeQueue();
        } else if (type == Type::COMMANDLIST_TYPE_COPY) {
            queue = m_deviceContext->transferQueue();
        }

        if (queue) {
            queueFamilyIndex = queue->familyIndex();
        }

        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;
        info.queueFamilyIndex = queueFamilyIndex;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(m_deviceContext->vulkanDevice(), &info, nullptr, &m_commandPool) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkCreateCommandPool";
            return false;
        }
    }

    if (type == Type::COMMANDLIST_TYPE_BUNDLE) {
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext = nullptr;
        info.commandPool = m_commandPool;
        info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        info.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_deviceContext->vulkanDevice(), &info, &m_commandBuffer) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkAllocateCommandBuffers";
            return false;
        }
    } else {
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext = nullptr;
        info.commandPool = m_commandPool;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_deviceContext->vulkanDevice(), &info, &m_commandBuffer) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkAllocateCommandBuffers";
            return false;
        }
    }

    return true;
}

void VulkanCommandList::dispose()
{
    if (m_commandBuffer) {
        vkFreeCommandBuffers(m_deviceContext->vulkanDevice(), m_commandPool, 1, &m_commandBuffer);
        m_commandBuffer = nullptr;
    }

    if (m_commandPool) {
        vkDestroyCommandPool(m_deviceContext->vulkanDevice(), m_commandPool, nullptr);
        m_commandPool = 0;
    }
}

void VulkanCommandList::begin()
{
    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritanceInfo.pNext = nullptr;
    inheritanceInfo.renderPass = 0;
    inheritanceInfo.subpass = 0;
    inheritanceInfo.framebuffer = 0;
    inheritanceInfo.occlusionQueryEnable = VK_FALSE;
    inheritanceInfo.queryFlags = 0;
    inheritanceInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = &inheritanceInfo;

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
        LN_LOG_ERROR << "Failed vkBeginCommandBuffer";
        return;
    }

    // TODO:
    //m_pFrameBuffer = nullptr;

    VkViewport dummyViewport = {};
    dummyViewport.width = 1;
    dummyViewport.height = 1;
    dummyViewport.minDepth = 0.0f;
    dummyViewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &dummyViewport);

    VkRect2D dummyScissor = {};
    vkCmdSetScissor(m_commandBuffer, 0, 1, &dummyScissor);

    float blendConstant[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    vkCmdSetBlendConstants(m_commandBuffer, blendConstant);

    vkCmdSetStencilReference(m_commandBuffer, VK_STENCIL_FRONT_AND_BACK, 0);
}

void VulkanCommandList::end()
{
    // TODO:
    //if (m_pFrameBuffer != nullptr)
    //{
    //    vkCmdEndRenderPass(m_commandBuffer);
    //    m_pFrameBuffer = nullptr;
    //}

    vkEndCommandBuffer(m_commandBuffer);
}

void VulkanCommandList::flush()
{
    VulkanQueue* queue = m_deviceContext->graphicsQueue();
    VkQueue vulkanQueue = queue->vulkanQueue();

    VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext = nullptr;
    info.waitSemaphoreCount = 0;
    info.pWaitSemaphores = nullptr;
    info.pWaitDstStageMask = &waitDstStageMask;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &m_commandBuffer;
    info.signalSemaphoreCount = 0;
    info.pSignalSemaphores = nullptr;

    vkQueueSubmit(vulkanQueue, 1, &info, 0);
    vkQueueWaitIdle(vulkanQueue);
}

//==============================================================================
// VulkanPipeline

VulkanPipeline::VulkanPipeline()
{
}

VulkanPipeline::~VulkanPipeline()
{
}

bool VulkanPipeline::init(VulkanDeviceContext* deviceContext, const IGraphicsDeviceContext::State& committed)
{
	m_deviceContext = deviceContext;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	VkPipelineColorBlendAttachmentState colorBlendAttachments[BlendStateDesc::MaxRenderTargets] = {};
	{
		const BlendStateDesc& state = committed.blendState;
		int attachmentsCount = 0;
		for (int i = 0; i < BlendStateDesc::MaxRenderTargets; i++) {
			colorBlendAttachments[i].blendEnable = (state.renderTargets[i].blendEnable) ? VK_TRUE : VK_FALSE;

			colorBlendAttachments[i].srcColorBlendFactor = LNBlendFactorToVkBlendFactor_Color(state.renderTargets[i].sourceBlend);
			colorBlendAttachments[i].dstColorBlendFactor = LNBlendFactorToVkBlendFactor_Color(state.renderTargets[i].destinationBlend);
			colorBlendAttachments[i].colorBlendOp = LNBlendOpToVkBlendOp(state.renderTargets[i].blendOp);

			colorBlendAttachments[i].srcAlphaBlendFactor = LNBlendFactorToVkBlendFactor_Alpha(state.renderTargets[i].sourceBlendAlpha);
			colorBlendAttachments[i].dstAlphaBlendFactor = LNBlendFactorToVkBlendFactor_Alpha(state.renderTargets[i].destinationBlendAlpha);
			colorBlendAttachments[i].alphaBlendOp = LNBlendOpToVkBlendOp(state.renderTargets[i].blendOpAlpha);

			colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

			attachmentsCount++;

			if (!state.independentBlendEnable) {
				break;
			}
		}

		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = attachmentsCount;
		colorBlending.pAttachments = colorBlendAttachments;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
	}

	VkPipelineRasterizationStateCreateInfo rasterizerInfo = {};
	{
		const RasterizerStateDesc& state = committed.rasterizerState;

		rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerInfo.depthClampEnable = VK_FALSE;
		rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizerInfo.polygonMode = LNFillModeToVkPolygonMode(state.fillMode);
		rasterizerInfo.cullMode = LNCullModeToVkCullMode(state.cullMode);
		rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // 右回り
		rasterizerInfo.depthBiasEnable = VK_FALSE;
		rasterizerInfo.depthBiasConstantFactor = 0.0f;
		rasterizerInfo.depthBiasClamp = 0.0f;
		rasterizerInfo.depthBiasSlopeFactor = 0.0f;
		rasterizerInfo.lineWidth = 1.0f;
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {};
	{
		const DepthStencilStateDesc& state = committed.depthStencilState;

		depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateInfo.pNext = nullptr;
		depthStencilStateInfo.flags = 0;
		depthStencilStateInfo.depthTestEnable = (state.depthTestFunc == ComparisonFunc::Never ? VK_FALSE : VK_TRUE);
		depthStencilStateInfo.depthWriteEnable = (state.depthWriteEnabled ? VK_TRUE : VK_FALSE);
		depthStencilStateInfo.depthCompareOp = LNComparisonFuncToVkCompareOp(state.depthTestFunc);
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateInfo.stencilTestEnable = (state.stencilEnabled ? VK_TRUE : VK_FALSE);

		depthStencilStateInfo.front.failOp = LNStencilOpToVkStencilOp(state.frontFace.stencilFailOp);
		depthStencilStateInfo.front.passOp = LNStencilOpToVkStencilOp(state.frontFace.stencilPassOp);
		depthStencilStateInfo.front.depthFailOp = LNStencilOpToVkStencilOp(state.frontFace.stencilDepthFailOp);
		depthStencilStateInfo.front.compareOp = LNComparisonFuncToVkCompareOp(state.frontFace.stencilFunc);
		depthStencilStateInfo.front.compareMask = UINT32_MAX;
		depthStencilStateInfo.front.writeMask = UINT32_MAX;
		depthStencilStateInfo.front.reference = state.stencilReferenceValue;

		depthStencilStateInfo.back.failOp = LNStencilOpToVkStencilOp(state.backFace.stencilFailOp);
		depthStencilStateInfo.back.passOp = LNStencilOpToVkStencilOp(state.backFace.stencilPassOp);
		depthStencilStateInfo.back.depthFailOp = LNStencilOpToVkStencilOp(state.backFace.stencilDepthFailOp);
		depthStencilStateInfo.back.compareOp = LNComparisonFuncToVkCompareOp(state.backFace.stencilFunc);
		depthStencilStateInfo.back.compareMask = UINT32_MAX;
		depthStencilStateInfo.back.writeMask = UINT32_MAX;
		depthStencilStateInfo.back.reference = state.stencilReferenceValue;

		depthStencilStateInfo.minDepthBounds = 0.0f;
		depthStencilStateInfo.maxDepthBounds = 1.0f;
	}

	VkPipelineMultisampleStateCreateInfo multisampleState;
	{
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.pNext = nullptr;
		multisampleState.flags = 0;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.minSampleShading = 0.0f;
		multisampleState.pSampleMask = nullptr;
		multisampleState.alphaToCoverageEnable = VK_FALSE;
		multisampleState.alphaToOneEnable = VK_FALSE;
	}

	// TODO:
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // TODO
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// viewport と scissor については DynamicState として Command で設定するため、 ここではダミーの値を登録しておく。
	VkPipelineViewportStateCreateInfo viewportState;
	VkViewport viewport;
	VkRect2D scissor;
	{
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = 1.0f;
		viewport.height = 1.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = 1;
		scissor.extent.height = 1;

		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;
		viewportState.flags = 0;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
	}

	//VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkRenderPass renderPass;
	if (!m_deviceContext->getVkRenderPass(committed.renderTargets.data(), committed.renderTargets.size(), committed.depthBuffer, &renderPass)) {
		return false;
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 0;    // TODO: まずはシェーダ無し //2;
	pipelineInfo.pStages = nullptr; // TODO: まずはシェーダ無し //shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizerInfo;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = 0; // TODO: まずは VertexDecl なし //pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(m_deviceContext->vulkanDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, m_deviceContext->vulkanAllocator(), &m_pipeline) != VK_SUCCESS) {
		LN_LOG_ERROR << "Failed vkCreateGraphicsPipelines";
		return false;
	}

	// command

	//VkViewport vp;
	//vp.x = rect.m_x;
	//vp.y = rect.m_y;
	//vp.width = rect.m_width;
	//vp.height = rect.m_height;
	//vp.minDepth = 0.0f;
	//vp.maxDepth = 1.0f;
	//vkCmdSetViewport(m_commandBuffer, 0, 1, &vp);

	//VkRect2D rc;
	//rc.offset.x = viewScissorRect.m_x;
	//rc.offset.y = viewScissorRect.m_y;
	//rc.extent.width = viewScissorRect.m_x + viewScissorRect.m_width;
	//rc.extent.height = viewScissorRect.m_y + viewScissorRect.m_height;
	//vkCmdSetScissor(m_commandBuffer, 0, 1, &rc);

	return true;
}

void VulkanPipeline::dispose()
{
}


//=============================================================================
// VulkanSwapChain

VulkanSwapChain::VulkanSwapChain()
{
}

VulkanSwapChain::~VulkanSwapChain()
{
}

bool VulkanSwapChain::init(VulkanDeviceContext* deviceContext, PlatformWindow* window, const SwapChainDesc& desc)
{
    m_deviceContext = deviceContext;
    m_desc = desc;

#if defined(LN_OS_WIN32)
    {
        HWND hWnd = (HWND)PlatformSupport::getWin32WindowHandle(window);
        HINSTANCE hInstance = ::GetModuleHandle(NULL);

        VkWin32SurfaceCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        info.pNext = nullptr;
        info.flags = 0;
        info.hinstance = hInstance;
        info.hwnd = hWnd;

        if (vkCreateWin32SurfaceKHR(m_deviceContext->vulkanInstance(), &info, nullptr, &m_surface) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkCreateWin32SurfaceKHR";
            return false;
        }
    }
#endif

    m_graphicsQueue = m_deviceContext->graphicsQueue();

    VkDevice vulkanDevice = m_deviceContext->vulkanDevice();
    VkPhysicalDevice vulkanPhysicalDevice = m_deviceContext->vulkanPhysicalDevice();

    // Check format
    m_imageFormat = VK_FORMAT_UNDEFINED;
    m_colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    {
        // 事前に取り出しておいた GraphicsQueue が、Present をサポートするかを確認する
        // Note: 丁寧にやるなら、もう一度すべての Queue を列挙して調べなおすのがよい。
        //       ただし、チュートリアルにもあるように、ほとんどのケースでは同じ Queue が選択される。
        //       https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface
        auto familyIndex = m_graphicsQueue->familyIndex();
        VkBool32 support = VK_FALSE;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(vulkanPhysicalDevice, familyIndex, m_surface, &support) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkGetPhysicalDeviceSurfaceSupportKHR";
            return false;
        }
        if (support == VK_FALSE) {
            LN_LOG_ERROR << "Failed vkGetPhysicalDeviceSurfaceSupportKHR unsupported";
            return false;
        }

        uint32_t srfaceFormatCount;
        if (vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanPhysicalDevice, m_surface, &srfaceFormatCount, nullptr) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkGetPhysicalDeviceSurfaceFormatsKHR";
            return false;
        }
        m_surfaceFormats.resize(srfaceFormatCount);
        if (vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanPhysicalDevice, m_surface, &srfaceFormatCount, m_surfaceFormats.data()) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkGetPhysicalDeviceSurfaceFormatsKHR";
            return false;
        }

        //bool found = false;

        auto nativeFormat = LNFormatToVkFormat(m_desc.Format);
        auto nativeColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        for (int i = 0; i < m_surfaceFormats.size(); i++) {
            if (nativeFormat == m_surfaceFormats[i].format &&
                nativeColorSpace == m_surfaceFormats[i].colorSpace) {
                m_imageFormat = m_surfaceFormats[i].format;
                m_colorSpace = m_surfaceFormats[i].colorSpace;
                //found = true;
                break;
            }
        }

        // VK_FORMAT_UNDEFINED のままでは vkCreateSwapchainKHR が強制終了する
        if (m_imageFormat == VK_FORMAT_UNDEFINED) {
            m_imageFormat = m_surfaceFormats[0].format;
            m_colorSpace = m_surfaceFormats[0].colorSpace;
        }

        //if (!found) {
        //    LN_LOG_ERROR << "Not found requested format";
        //    return false;
        //}
    }

    // Check buffer caps
    {
        VkSurfaceCapabilitiesKHR capabilities;
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanPhysicalDevice, m_surface, &capabilities) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkGetPhysicalDeviceSurfaceFormatsKHR";
            return false;
        }

        if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
            m_preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        } else {
            m_preTransform = capabilities.currentTransform;
        }

        if (capabilities.maxImageCount < m_desc.BufferCount) {
            LN_LOG_ERROR << "Invalid buffer count";
            return false;
        }

        if (capabilities.maxImageExtent.width < m_desc.Width) {
            m_desc.Width = capabilities.maxImageExtent.width;
        }
        if (capabilities.maxImageExtent.height < m_desc.Height) {
            m_desc.Height = capabilities.maxImageExtent.height;
        }
    }

    // Select present mode
    {
        m_presentMode = VK_PRESENT_MODE_FIFO_KHR;

        uint32_t presentModeCount;
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanPhysicalDevice, m_surface, &presentModeCount, nullptr) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkGetPhysicalDeviceSurfacePresentModesKHR";
            return false;
        }
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanPhysicalDevice, m_surface, &presentModeCount, presentModes.data()) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkGetPhysicalDeviceSurfacePresentModesKHR";
            return false;
        }

        bool found = false;
        for (uint32_t i = 0; i < presentModeCount; i++) {
            if (m_desc.SyncInterval == 0) {
                if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                    m_presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                    found = true;
                    break;
                }
            } else if (m_desc.SyncInterval == -1) {
                if (presentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
                    m_presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
                    found = true;
                    break;
                }
            } else {
                if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR) {
                    m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            LN_LOG_ERROR << "Not found present mode";
            return false;
        }
    }

    // Create swap chain
    {
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = m_surface;
        createInfo.minImageCount = m_desc.BufferCount;
        createInfo.imageFormat = m_imageFormat;
        createInfo.imageColorSpace = m_colorSpace;
        createInfo.imageExtent = {m_desc.Width, m_desc.Height};
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = m_preTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = m_presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = 0;

        if (vkCreateSwapchainKHR(vulkanDevice, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkCreateSwapchainKHR";
            return false;
        }
    }

    // Get swap chain images
    {
        uint32_t chainCount;
        if (vkGetSwapchainImagesKHR(vulkanDevice, m_swapChain, &chainCount, nullptr) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkGetSwapchainImagesKHR";
            return false;
        }
        if (chainCount != m_desc.BufferCount) {
            LN_LOG_ERROR << "Invalid chain count";
            return false;
        }

        m_images.resize(chainCount);
        m_imageViews.resize(chainCount);

        if (vkGetSwapchainImagesKHR(vulkanDevice, m_swapChain, &chainCount, m_images.data()) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkGetSwapchainImagesKHR";
            return false;
        }

        // ここで取り出した Image は VkSwapchainKHR が破棄されると自動的にクリーンアップされるので、クリーンアップコードを追加する必要はない。
    }

    // イメージビューを生成.
    {
        VkImageSubresourceRange range = {};
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel = 0;
        range.layerCount = 1;
        range.baseArrayLayer = 0;
        range.levelCount = m_desc.MipLevels;

        for (auto i = 0u; i < m_desc.BufferCount; ++i) {
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.pNext = nullptr;
            viewInfo.flags = 0;
            viewInfo.image = m_images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_imageFormat;
            viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            viewInfo.subresourceRange = range;

            if (vkCreateImageView(vulkanDevice, &viewInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS) {
                LN_LOG_ERROR << "Failed vkCreateImageView";
                return false;
            }
        }
    }

    // バックバッファに Texture としてアクセスできるようにラップしたインスタンスを作っておく
    {
        m_buffers.resize(m_desc.BufferCount);
        for (auto i = 0u; i < m_desc.BufferCount; ++i) {
            auto texture = makeRef<VulkanRenderTargetTexture>();
            if (!texture->init(m_deviceContext, m_desc, m_images[i], m_imageViews[i])) {
                return false;
            }
            m_buffers[i] = texture;
        }
    }

    // Change image layout
    // https://sites.google.com/site/monshonosuana/vulkan/vulkan_002
    {
        auto commandBuffer = makeRef<VulkanCommandList>();
        if (!commandBuffer->init(m_deviceContext, VulkanCommandList::Type::COMMANDLIST_TYPE_DIRECT)) {
            return false;
        }

        commandBuffer->begin();
        VkCommandBuffer cmdBuffer = commandBuffer->vulkanCommandBuffer();

        for (auto i = 0u; i < m_desc.BufferCount; ++i) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.pNext = nullptr;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.srcQueueFamilyIndex = 0;
            barrier.dstQueueFamilyIndex = 0;
            barrier.image = m_buffers[i]->vulkanImage();

            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.layerCount = m_buffers[i]->desc().DepthOrArraySize;
            barrier.subresourceRange.levelCount = m_buffers[i]->desc().MipLevels;

            vkCmdPipelineBarrier(
                cmdBuffer,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier);
        }

        commandBuffer->end();
        commandBuffer->flush();
        commandBuffer->dispose();
    }

    // Get current backbuffer index
    {
        uint32_t index = m_graphicsQueue->currentBufferIndex();
        VkSemaphore semaphore = m_graphicsQueue->vulkanWaitSemaphore(index);
        VkFence fence = m_graphicsQueue->vulkanFence(index);
        if (vkAcquireNextImageKHR(vulkanDevice, m_swapChain, UINT64_MAX, semaphore, fence, &m_currentBufferIndex) != VK_SUCCESS) {
            LN_LOG_ERROR << "Failed vkAcquireNextImageKHR";
            return false;
        }

        vkWaitForFences(vulkanDevice, 1, &fence, VK_FALSE, UINT64_MAX);
        vkResetFences(vulkanDevice, 1, &fence);
    }

    return true;
}

void VulkanSwapChain::dispose()
{
    if (m_swapChain) {
        vkDestroySwapchainKHR(m_deviceContext->vulkanDevice(), m_swapChain, nullptr);
        m_swapChain = 0;
    }

    if (m_surface) {
        vkDestroySurfaceKHR(m_deviceContext->vulkanInstance(), m_surface, nullptr);
        m_surface = 0;
    }

    ISwapChain::dispose();
}

ITexture* VulkanSwapChain::getColorBuffer() const
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

bool VulkanSwapChain::present()
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // 実際に Presentation が行われる前に待機するセマフォを指定する。
    // これは VkSubmitInfo に指定したものと同一とする。
    //presentInfo.waitSemaphoreCount = 1;
    //presentInfo.pWaitSemaphores = signalSemaphores;

    return true;
}

//==============================================================================
// VulkanVertexDeclaration

VulkanVertexDeclaration::VulkanVertexDeclaration()
{
}

VulkanVertexDeclaration::~VulkanVertexDeclaration()
{
}

bool VulkanVertexDeclaration::init(const VertexElement* elements, int elementsCount)
{
    uint32_t bindingCount = 0;
    for (int i = 0; i < elementsCount; i++) {
        bindingCount = std::max(bindingCount, elements[i].StreamIndex);
    }
    m_bindings.resize(bindingCount);

    uint32_t loc = 0;
    for (int i = 0; i < elementsCount; i++) {
        VkVertexInputAttributeDescription attr;
        attr.location = loc; // TODO: ひとまず先頭から1ずつ
        attr.binding = elements[i].StreamIndex;
        attr.format = LNVertexElementTypeToVkFormat(elements[i].Type);
        attr.offset = m_bindings[attr.binding].stride;
        m_bindings[attr.binding].stride += GraphicsHelper::getVertexElementTypeSize(elements[i].Type);

		loc++;

        // TODO: Lumino のシェーダとしては、location と Semantics の対応を固定してもいいかもしれない。
        // たとえば、location=0 は POSITION0 とか。
    }

    return true;
}

void VulkanVertexDeclaration::dispose()
{
    IVertexDeclaration::dispose();
}

//==============================================================================
// VulkanVertexBuffer

VulkanVertexBuffer::VulkanVertexBuffer()
{
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
}

void VulkanVertexBuffer::init(GraphicsResourceUsage usage, size_t bufferSize, const void* initialData)
{
    LN_NOTIMPLEMENTED();
}

void VulkanVertexBuffer::dispose()
{
    IVertexBuffer::dispose();
}

size_t VulkanVertexBuffer::getBytesSize()
{
    LN_NOTIMPLEMENTED();
    return 0;
}

GraphicsResourceUsage VulkanVertexBuffer::usage() const
{
    LN_NOTIMPLEMENTED();
    return GraphicsResourceUsage::Static;
}

void VulkanVertexBuffer::setSubData(size_t offset, const void* data, size_t length)
{
    LN_NOTIMPLEMENTED();
}

void* VulkanVertexBuffer::map()
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

void VulkanVertexBuffer::unmap()
{
    LN_NOTIMPLEMENTED();
}

//==============================================================================
// VulkanIndexBuffer

VulkanIndexBuffer::VulkanIndexBuffer()
{
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
}

void VulkanIndexBuffer::init(GraphicsResourceUsage usage, IndexBufferFormat format, int indexCount, const void* initialData)
{
    LN_NOTIMPLEMENTED();
}

void VulkanIndexBuffer::dispose()
{
    LN_NOTIMPLEMENTED();
    IIndexBuffer::dispose();
}

size_t VulkanIndexBuffer::getBytesSize()
{
    LN_NOTIMPLEMENTED();
    return 0;
}

GraphicsResourceUsage VulkanIndexBuffer::usage() const
{
    LN_NOTIMPLEMENTED();
    return GraphicsResourceUsage::Static;
}

void VulkanIndexBuffer::setSubData(size_t offset, const void* data, size_t length)
{
    LN_NOTIMPLEMENTED();
}

void* VulkanIndexBuffer::map()
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

void VulkanIndexBuffer::unmap()
{
    LN_NOTIMPLEMENTED();
}

//=============================================================================
// VulkanTextureBase

VulkanTextureBase::VulkanTextureBase()
{
}

VulkanTextureBase::~VulkanTextureBase()
{
}

//=============================================================================
// VulkanTexture2D

VulkanTexture2D::VulkanTexture2D()
{
}

VulkanTexture2D::~VulkanTexture2D()
{
}

bool VulkanTexture2D::init(uint32_t width, uint32_t height, TextureFormat requestFormat, bool mipmap, const void* initialData)
{
    LN_NOTIMPLEMENTED();
    return true;
}

void VulkanTexture2D::dispose()
{
    VulkanTextureBase::dispose();
}

DeviceTextureType VulkanTexture2D::type() const
{
    return DeviceTextureType::Texture2D;
}

void VulkanTexture2D::readData(void* outData)
{
    LN_UNREACHABLE();
}

const SizeI& VulkanTexture2D::realSize()
{
    LN_NOTIMPLEMENTED();
    return SizeI::Zero;
}

TextureFormat VulkanTexture2D::getTextureFormat() const
{
    LN_NOTIMPLEMENTED();
    return TextureFormat::Unknown;
}

void VulkanTexture2D::setSubData(int x, int y, int width, int height, const void* data, size_t dataSize)
{
    LN_NOTIMPLEMENTED();
}

void VulkanTexture2D::setSubData3D(int x, int y, int z, int width, int height, int depth, const void* data, size_t dataSize)
{
    LN_UNREACHABLE();
}

//=============================================================================
// VulkanTexture3D

VulkanTexture3D::VulkanTexture3D()
{
}

VulkanTexture3D::~VulkanTexture3D()
{
}

void VulkanTexture3D::init(uint32_t width, uint32_t height, uint32_t depth, TextureFormat requestFormat, bool mipmap, const void* initialData)
{
    LN_NOTIMPLEMENTED();
}

void VulkanTexture3D::dispose()
{
    LN_NOTIMPLEMENTED();
    VulkanTextureBase::dispose();
}

DeviceTextureType VulkanTexture3D::type() const
{
    return DeviceTextureType::Texture3D;
}

void VulkanTexture3D::readData(void* outData)
{
    LN_UNREACHABLE();
}

const SizeI& VulkanTexture3D::realSize()
{
    LN_NOTIMPLEMENTED();
    return SizeI::Zero;
}

TextureFormat VulkanTexture3D::getTextureFormat() const
{
    LN_NOTIMPLEMENTED();
    return TextureFormat::Unknown;
}

void VulkanTexture3D::setSubData(int x, int y, int width, int height, const void* data, size_t dataSize)
{
    LN_UNREACHABLE();
}

void VulkanTexture3D::setSubData3D(int x, int y, int z, int width, int height, int depth, const void* data, size_t dataSize)
{
    LN_NOTIMPLEMENTED();
}

//=============================================================================
// VulkanRenderTargetTexture

VulkanRenderTargetTexture::VulkanRenderTargetTexture()
{
}

VulkanRenderTargetTexture::~VulkanRenderTargetTexture()
{
}

bool VulkanRenderTargetTexture::init(VulkanDeviceContext* deviceContext, const VulkanSwapChain::SwapChainDesc& desc, VkImage image, VkImageView view)
{
	m_deviceContext = deviceContext;
	m_isExternal = true;

	m_image = image;
	m_imageView = view;
	m_imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	m_deviceMemory = 0;
	//m_Desc.Dimension = RESOURCE_DIMENSION_TEXTURE2D;
	m_desc.Width = desc.Width;
	m_desc.Height = desc.Height;
	m_desc.DepthOrArraySize = 1;
	//m_Desc.Format = desc.Format;
	m_desc.MipLevels = desc.MipLevels;
	//m_Desc.SampleCount = desc.SampleCount;
	//m_Desc.Layout = RESOURCE_LAYOUT_OPTIMAL;
	//m_Desc.InitState = RESOURCE_STATE_UNKNOWN;
	//m_Desc.HeapProperty.Type = HEAP_TYPE_DEFAULT;
	//m_Desc.HeapProperty.CpuPageProperty = CPU_PAGE_PROPERTY_NOT_AVAILABLE;

	vkGetImageMemoryRequirements(m_deviceContext->vulkanDevice(), image, &m_memoryRequirements);

	return true;
}

void VulkanRenderTargetTexture::init(VulkanDeviceContext* context, uint32_t width, uint32_t height, TextureFormat requestFormat, bool mipmap)
{
	m_deviceContext = context;
    LN_NOTIMPLEMENTED();
}

void VulkanRenderTargetTexture::dispose()
{
	if (m_deviceContext) {
		m_deviceContext->frameBufferCache().invalidateRenderTarget(this);
	}

	if (!m_isExternal) {
		if (m_image) {
			vkDestroyImage(m_deviceContext->vulkanDevice(), m_image, nullptr);
			m_image = 0;
		}

		if (m_deviceMemory) {
			vkFreeMemory(m_deviceContext->vulkanDevice(), m_deviceMemory, nullptr);
			m_deviceMemory = 0;
		}
	}
	else {
		m_image = 0;
		m_deviceMemory = 0;
	}

	memset(&m_memoryRequirements, 0, sizeof(m_memoryRequirements));
	//memset(&m_Desc, 0, sizeof(m_Desc));

    LN_NOTIMPLEMENTED();
    VulkanTextureBase::dispose();
}

DeviceTextureType VulkanRenderTargetTexture::type() const
{
    return DeviceTextureType::RenderTarget;
}

void VulkanRenderTargetTexture::readData(void* outData)
{
    LN_NOTIMPLEMENTED();
}

const SizeI& VulkanRenderTargetTexture::realSize()
{
    LN_NOTIMPLEMENTED();
    return SizeI::Zero;
}

TextureFormat VulkanRenderTargetTexture::getTextureFormat() const
{
    LN_NOTIMPLEMENTED();
    return TextureFormat::Unknown;
}

void VulkanRenderTargetTexture::setSubData(int x, int y, int width, int height, const void* data, size_t dataSize)
{
    LN_UNREACHABLE();
}

void VulkanRenderTargetTexture::setSubData3D(int x, int y, int z, int width, int height, int depth, const void* data, size_t dataSize)
{
    LN_UNREACHABLE();
}

//=============================================================================
// VulkanDepthBuffer

VulkanDepthBuffer::VulkanDepthBuffer()
{
}

VulkanDepthBuffer::~VulkanDepthBuffer()
{
}

void VulkanDepthBuffer::init(VulkanDeviceContext* context, uint32_t width, uint32_t height)
{
	m_deviceContext = context;
    LN_NOTIMPLEMENTED();
}

void VulkanDepthBuffer::dispose()
{
	if (m_deviceContext) {
		m_deviceContext->frameBufferCache().invalidateDepthBuffer(this);
	}

    IDepthBuffer::dispose();
}

//=============================================================================
// VulkanSamplerState

VulkanSamplerState::VulkanSamplerState()
{
}

VulkanSamplerState::~VulkanSamplerState()
{
}

void VulkanSamplerState::init(const SamplerStateData& desc)
{
    LN_NOTIMPLEMENTED();
}

void VulkanSamplerState::dispose()
{
    LN_NOTIMPLEMENTED();
    ISamplerState::dispose();
}

//=============================================================================
// VulkanShaderPass

VulkanShaderPass::VulkanShaderPass()
{
}

VulkanShaderPass::~VulkanShaderPass()
{
}

void VulkanShaderPass::init(VulkanDeviceContext* context, const byte_t* vsCode, int vsCodeLen, const byte_t* fsCode, int fsCodeLen, ShaderCompilationDiag* diag)
{
    LN_NOTIMPLEMENTED();
}

void VulkanShaderPass::dispose()
{
    IShaderPass::dispose();
}

int VulkanShaderPass::getUniformCount() const
{
    LN_NOTIMPLEMENTED();
    return 0;
}

IShaderUniform* VulkanShaderPass::getUniform(int index) const
{
    // TODO: 必要？
    LN_NOTIMPLEMENTED();
    return nullptr;
}

void VulkanShaderPass::setUniformValue(int index, const void* data, size_t size)
{
    LN_NOTIMPLEMENTED();
}

int VulkanShaderPass::getUniformBufferCount() const
{
    LN_NOTIMPLEMENTED();
    return 0;
}

IShaderUniformBuffer* VulkanShaderPass::getUniformBuffer(int index) const
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

IShaderSamplerBuffer* VulkanShaderPass::samplerBuffer() const
{
    LN_NOTIMPLEMENTED();
    return 0;
}

//=============================================================================
// VulkanShaderUniformBuffer

VulkanShaderUniformBuffer::VulkanShaderUniformBuffer()
{
}

VulkanShaderUniformBuffer::~VulkanShaderUniformBuffer()
{
}

void VulkanShaderUniformBuffer::init()
{
    LN_NOTIMPLEMENTED();
}

void VulkanShaderUniformBuffer::dispose()
{
    IShaderUniformBuffer::dispose();
}

const std::string& VulkanShaderUniformBuffer::name() const
{
    LN_NOTIMPLEMENTED();
    return m_name;
}

int VulkanShaderUniformBuffer::getUniformCount() const
{
    LN_NOTIMPLEMENTED();
    return 0;
}

IShaderUniform* VulkanShaderUniformBuffer::getUniform(int index) const
{
    LN_NOTIMPLEMENTED();
    return nullptr;
}

size_t VulkanShaderUniformBuffer::bufferSize() const
{
    LN_NOTIMPLEMENTED();
    return 0;
}

void VulkanShaderUniformBuffer::setData(const void* data, size_t size)
{
    LN_NOTIMPLEMENTED();
}

//=============================================================================
// VulkanShaderUniform

VulkanShaderUniform::VulkanShaderUniform()
{
}

VulkanShaderUniform::~VulkanShaderUniform()
{
}

void VulkanShaderUniform::init()
{
    LN_NOTIMPLEMENTED();
}

void VulkanShaderUniform::dispose()
{
    IShaderUniform::dispose();
}

//=============================================================================
// VulkanLocalShaderSamplerBuffer

VulkanLocalShaderSamplerBuffer::VulkanLocalShaderSamplerBuffer()
{
}

VulkanLocalShaderSamplerBuffer::~VulkanLocalShaderSamplerBuffer()
{
}

void VulkanLocalShaderSamplerBuffer::init()
{
}

void VulkanLocalShaderSamplerBuffer::dispose()
{
}

int VulkanLocalShaderSamplerBuffer::registerCount() const
{
    LN_NOTIMPLEMENTED();
    return 0;
}

const std::string& VulkanLocalShaderSamplerBuffer::getTextureRegisterName(int registerIndex) const
{
    LN_NOTIMPLEMENTED();
    return std::string();
}

const std::string& VulkanLocalShaderSamplerBuffer::getSamplerRegisterName(int registerIndex) const
{
    LN_NOTIMPLEMENTED();
    return std::string();
}

void VulkanLocalShaderSamplerBuffer::setTexture(int registerIndex, ITexture* texture)
{
    LN_NOTIMPLEMENTED();
}

void VulkanLocalShaderSamplerBuffer::setSamplerState(int registerIndex, ISamplerState* state)
{
    LN_NOTIMPLEMENTED();
}

} // namespace detail
} // namespace ln
