﻿/*
	UIContext って何者？
		アプリ内のUIシステムの境界。
		例えば、普通に使う2DのUIと、3Dワールド内に独立したUIを配置したいとき。
		3Dワールド内のUIでモーダルウィンドウ出しているとき、2Dのが操作不能になるのはちょっといただけない。
*/
#include "Internal.h"
#include <Lumino/UI/UILayoutView.h>
#include <Lumino/UI/UIContext.h>
#include <Lumino/UI/UIFrameWindow.h>
#include <Lumino/UI/UIStyle.h>
#include "../Graphics/GraphicsManager.h"
#include <Lumino/Graphics/GraphicsContext.h>
#include "UIManager.h"
#include "UIHelper.h"

LN_NAMESPACE_BEGIN

//==============================================================================
// UIContext
//==============================================================================

//------------------------------------------------------------------------------
UIContext* UIContext::getMainContext()
{
	return detail::UIManager::getInstance()->getMainWindow()->GetMainUIContext();
}

//------------------------------------------------------------------------------
UIContext::UIContext()
	: m_manager(nullptr)
	, m_rootStyleTable(nullptr)
	//, m_mainWindowView(nullptr)
	, m_focusElement(nullptr)
{
}

//------------------------------------------------------------------------------
UIContext::~UIContext()
{
	//LN_SAFE_RELEASE(m_mainWindowView);
	LN_SAFE_RELEASE(m_rootStyleTable);
}

//------------------------------------------------------------------------------
void UIContext::initialize(detail::UIManager* manager)
{
	m_manager = manager;

	LN_REFOBJ_SET(m_rootStyleTable, m_manager->GetDefaultStyleTable());

	//m_mainWindowView = LN_NEW UILayoutView();
	//m_mainWindowView->initialize(this, m_manager->getMainWindow()->GetPlatformWindow());
}

//------------------------------------------------------------------------------
void UIContext::SetFocusElement(UIElement* element)
{
	if (element != nullptr)
	{
		if (LN_CHECK_STATE(element->IsFocusable())) return;
	}

	UIElement* focusedBranchRoot = UIHelper::FindVisualAncestor(element, true, [](UIElement* e) { return e->HasFocus() || e->GetSpcialUIElementType() == detail::SpcialUIElementType::LayoutRoot; });
	if (LN_CHECK_STATE(focusedBranchRoot != nullptr)) return;
	
	if (m_focusElement != nullptr)
	{
		if (m_focusElement->IsFocusable() && m_focusElement->HasFocus()) m_focusElement->CallOnLostFocus();
		UIHelper::FindVisualAncestor(m_focusElement, false, [focusedBranchRoot](UIElement* e)
		{
			if (e == focusedBranchRoot) return true;
			if (e->IsFocusable() && e->HasFocus()) e->CallOnLostFocus();
			return false;
		});
	}

	if (element->IsFocusable() && !element->HasFocus()) element->CallOnGotFocus();
	UIHelper::FindVisualAncestor(element, false, [focusedBranchRoot](UIElement* e)
	{
		if (e == focusedBranchRoot) return true;
		if (e->IsFocusable() && !e->HasFocus()) e->CallOnGotFocus();
		return false;
	});

	// 初回用
	if (!focusedBranchRoot->HasFocus())
	{
		focusedBranchRoot->CallOnGotFocus();
	}

	m_focusElement = element;


	//if (m_focusElement != element)
	//{
	//	if (m_focusElement != nullptr)
	//	{
	//		m_focusElement->CallOnLostFocus();
	//	}

	//	m_focusElement = element;

	//	if (m_focusElement != nullptr)
	//	{
	//		m_focusElement->CallOnGotFocus();
	//	}
	//}
}

//------------------------------------------------------------------------------
void UIContext::InjectElapsedTime(float elapsedTime)
{
}

//------------------------------------------------------------------------------
//void UIContext::render()
//{
//	//auto* g = m_manager->getGraphicsManager()->GetGraphicsContext();
//	//auto* d = g->BeginDrawingContext();
//
//	//d->SetViewProjection(Matrix::Identity, Matrix::Perspective2DLH(640, 480, 0, 1));
//
//	//d->DrawRectangle(RectF(10, 10, 20, 30), ColorF::Red);
//
//	//g->Flush();
//}

LN_NAMESPACE_END
