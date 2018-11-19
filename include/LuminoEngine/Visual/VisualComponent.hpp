﻿
#pragma once
#include "Common.hpp"
#include "../Rendering/RenderingContext.hpp"
#include "../Scene/Component.hpp"

namespace ln {
namespace detail {
class GeometryStageParameters;
class BuiltinEffectData;
}

class VisualComponent
	: public Component
{
    LN_OBJECT;
public:
	/** 可視状態を設定します。false の場合、コンポーネントの描画は行われません。(default: true) */
	LN_METHOD(Property)
	void setVisible(bool value) { m_isVisible = value; }

	/** 可視状態を取得します。*/
	LN_METHOD(Property)
	bool isVisible() const { return m_isVisible; }

    /** 合成方法を設定します。(default: BlendMode::Normal) */
    void setBlendMode(const Optional<BlendMode>& mode);

    /** 合成方法を取得します。*/
    const Optional<BlendMode>& blendMode() const;

    /** 不透明度を設定します。(default: 1.0) */
    void setOpacity(float value);

    /** 不透明度を取得します。 */
    float opacity() const;

protected:

LN_CONSTRUCT_ACCESS:
    VisualComponent();
	virtual ~VisualComponent();
	void initialize();

private:
    virtual void render(RenderingContext* context) override;

    std::unique_ptr<detail::GeometryStageParameters> m_geometryStageParameters;
    std::unique_ptr<detail::BuiltinEffectData> m_builtinEffectData;
    bool m_isVisible;
};

} // namespace ln
