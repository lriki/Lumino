﻿
#pragma once
#include "UIControl.h"

LN_NAMESPACE_BEGIN

/**
	@brief		
*/
class UIWindow
	: public UIControl
{
	LN_TR_REFLECTION_TYPEINFO_DECLARE();
public:
	static RefPtr<UIWindow> create();

protected:
	virtual void OnRoutedEvent(UIEventArgs* e) override;
	virtual void OnLayoutPanelChanged(UILayoutPanel* newPanel) override;
	virtual void OnRender(DrawingContext* g) override;

LN_CONSTRUCT_ACCESS:
	UIWindow();
	virtual ~UIWindow();
	void initialize();

private:
	Vector2	m_dragStartWindowPosition;
	Vector2	m_dragStartLocalPosition;
	bool	m_isDragging;
};

LN_NAMESPACE_END
