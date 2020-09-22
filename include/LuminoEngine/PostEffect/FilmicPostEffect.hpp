﻿#pragma once
#include <LuminoEngine/PostEffect/SSRPostEffect.hpp>
#include <LuminoEngine/PostEffect/BloomPostEffect.hpp>
#include <LuminoEngine/PostEffect/DepthOfFieldPostEffect.hpp>
#include <LuminoEngine/PostEffect/TonemapPostEffect.hpp>

namespace ln {
class SamplerState;
namespace detail { class FilmicPostEffectInstance; }

class FilmicPostEffect
    : public PostEffect
{
public:
	/** アンチエイリアスの有無を設定します。(default: false) */
	LN_METHOD(Property)
	void setAntialiasEnabled(bool value) { m_antialiasEnabled = value; }

	/** アンチエイリアスの有無を取得します。 */
	LN_METHOD(Property)
	bool isAntialiasEnabled() const { return m_antialiasEnabled; }
    
	/** SSR (Screen Space Reflection) の有無を設定します。(default: false) */
	LN_METHOD(Property)
	void setSSREnabled(bool value) { m_ssrEnabled = value; }

	/** SSR の有無を取得します。 */
	LN_METHOD(Property)
	bool isSSREnabled() const { return m_ssrEnabled; }
    
	/** SSAO (Screen Space Ambient Occlusion) の有無を設定します。(default: false) */
	LN_METHOD(Property)
	void setSSAOEnabled(bool value) { m_ssaoEnabled = value; }

	/** SSAO の有無を取得します。 */
	LN_METHOD(Property)
	bool isSSAOEnabled() const { return m_ssaoEnabled; }
    
	/** ブルームエフェクトの有無を設定します。(default: false) */
	LN_METHOD(Property)
	void setBloomEnabled(bool value) { m_bloomEnabled = value; }

	/** ブルームエフェクトの有無を取得します。 */
	LN_METHOD(Property)
	bool isBloomEnabled() const { return m_bloomEnabled; }
    
	/** 被写界深度の有無を設定します。(default: false) */
	LN_METHOD(Property)
	void setDOFEnabled(bool value) { m_dofEnabled = value; }

	/** 被写界深度の有無を取得します。 */
	LN_METHOD(Property)
	bool isDOFEnabled() const { return m_dofEnabled; }
    
	/** トーンマッピングの有無を設定します。(default: false) */
	LN_METHOD(Property)
	void setTonemapEnabled(bool value) { m_tonemapEnabled = value; }

	/** トーンマッピングの有無を取得します。 */
	LN_METHOD(Property)
	bool isTonemapEnabled() const { return m_tonemapEnabled; }
    
	/** ビネットエフェクトの有無を設定します。(default: false) */
	LN_METHOD(Property)
	void setVignetteEnabled(bool value) { m_vignetteEnabled = value; }

	/** ビネットエフェクトの有無を取得します。 */
	LN_METHOD(Property)
	bool isVignetteEnabled() const { return m_vignetteEnabled; }
    
	/** ガンマ補正の有無を設定します。(default: false) */
	LN_METHOD(Property)
	void setGammaEnabled(bool value) { m_gammaEnabled = value; }

	/** ガンマ補正の有無を取得します。 */
	LN_METHOD(Property)
	bool isGammaEnabled() const { return m_gammaEnabled; }

protected:
    virtual Ref<PostEffectInstance> onCreateInstance() override;

LN_CONSTRUCT_ACCESS:
    FilmicPostEffect();
    void init();

private:
    float m_luminosityThreshold;
    float m_bloomStrength;
    float m_bloomRadius;

    bool m_antialiasEnabled;
    bool m_ssrEnabled;
    bool m_ssaoEnabled;
    bool m_bloomEnabled;
    bool m_dofEnabled;
    bool m_tonemapEnabled;
    bool m_vignetteEnabled;
	bool m_gammaEnabled;

    friend class detail::FilmicPostEffectInstance;
};

namespace detail {

class FilmicPostEffectInstance
    : public PostEffectInstance
{
public:


protected:
    bool onRender(RenderingContext* context, RenderTargetTexture* source, RenderTargetTexture* destination) override;

LN_CONSTRUCT_ACCESS:
    FilmicPostEffectInstance();
    bool init(FilmicPostEffect* owner);

private:
    FilmicPostEffect* m_owner;
    Ref<Material> m_integrationMaterial;
    Ref<Material> m_ssaoMaterial;
    Ref<SamplerState> m_samplerState;
    SSRPostEffectCore m_ssrEffect;
    BloomPostEffectCore m_bloomEffect;
    DepthOfFieldPostEffectCore m_dofEffect;

#if 1
#else
    bool m_antialiasEnabled = true;
    bool m_ssrEnabled = false;
    bool m_ssaoEnabled = false;
    bool m_bloomEnabled = false;
    bool m_dofEnabled = false;
    bool m_tonemapEnabled = true;
    bool m_vignetteEnabled = true;
    bool m_gammaEnabled = true;
#endif
};

} // namespace detail
} // namespace ln

