﻿#pragma once
#include "UILayoutElement.hpp"

namespace ln {
class UIRenderingContext;

class UIElement
	: public Object
    , public detail::ILayoutElement
{
public:
    UIElement();
	void initialize();

protected:

	/** この要素内の子ビジュアル要素の数を取得します。 */
	virtual int getVisualChildrenCount() const;

	/** 子ビジュアル要素を取得します。奥にある要素が先、手前にある要素が後になります。(Zオーダーやアクティブ状態の考慮は実装側で行うこと) */
	virtual UIElement* getVisualChild(int index) const;

	virtual void onRender(UIRenderingContext* context);


LN_INTERNAL_ACCESS:
	void render(UIRenderingContext* context);

private:
	
};

} // namespace ln

