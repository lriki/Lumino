﻿#pragma once
#include "SceneRenderer.hpp"
#include "LightClusters.hpp"

namespace ln {
namespace detail {

#if 0

struct FogParams
{
	Color	color;
	float	density = 0.0f;
};

class ClusteredShadingGeometryRenderingPass
	: public SceneRendererPass
{
public:
	ClusteredShadingGeometryRenderingPass();
	virtual ~ClusteredShadingGeometryRenderingPass();
	void initialize();

	//virtual Shader* getDefaultShader() const override;
	virtual void selectElementRenderingPolicy(DrawElement* element, const RenderStageFinalData& stageData, ElementRenderingPolicy* outPolicy) override;

	//virtual void onBeginPass(DefaultStatus* defaultStatus, RenderView* renderView) override;

private:
	Ref<Shader>					m_defaultShader;
	ShaderTechnique*			m_defaultShaderTechnique;
	Ref<Shader>					m_unLightingShader;
	ShaderTechnique*			m_unLightingShaderTechnique;
};

class DepthPrepass
	: public SceneRendererPass
{
public:
	DepthPrepass();
	virtual ~DepthPrepass();
	void initialize();

	virtual void selectElementRenderingPolicy(DrawElement* element, const RenderStageFinalData& stageData, ElementRenderingPolicy* outPolicy) override;
	virtual void onBeginPass(DefaultStatus* defaultStatus, RenderView* renderView) override;

public:	// TODO:
	Ref<Shader>					m_defaultShader;
	Ref<RenderTargetTexture>	m_depthMap;
};

#if 0
class ShadowCasterPass
	: public RenderingPass2
{
public:
	CameraInfo	view;

	ShadowCasterPass();
	virtual ~ShadowCasterPass();
	void initialize();

	//virtual Shader* getDefaultShader() const override;

	virtual void selectElementRenderingPolicy(DrawElement* element, const RenderStageFinalData& stageData, ElementRenderingPolicy* outPolicy) override;

	virtual void onBeginPass(DefaultStatus* defaultStatus, RenderView* renderView) override;

	virtual void overrideCameraInfo(detail::CameraInfo* cameraInfo) override;

protected:
	//virtual ShaderPass* selectShaderPass(Shader* shader) override;

public:	// TODO:
	Ref<Shader>		m_defaultShader;
	Ref<RenderTargetTexture>	m_shadowMap;
};
#endif


class ClusteredShadingSceneRenderer
	: public SceneRenderer
{
public:
	ClusteredShadingSceneRenderer();
	virtual ~ClusteredShadingSceneRenderer();
	void initialize(GraphicsManager* manager);
	//void setSceneGlobalRenderSettings(const SceneGlobalRenderSettings& settings) { m_renderSettings = settings; }
	void setFogParams(const FogParams& params) { m_fogParams = params; }
	DepthPrepass* getDepthPrepass() const { return m_depthPrepass; }

protected:
	virtual void collect(RenderingPass2* pass, const detail::CameraInfo& cameraInfo) override;
	virtual void prepare() override;
	virtual void onCollectLight(DynamicLightInfo* light) override;
	virtual void onShaderPassChainging(ShaderPass* pass) override;

private:
	LightClusters				m_lightClusters;
	//SceneGlobalRenderSettings	m_renderSettings;
	FogParams					m_fogParams;
	Ref<DepthPrepass>			m_depthPrepass;
};

#endif

} // namespace detail
} // namespace ln
