﻿
#include "Internal.h"
#include <Lumino/UI/UIControl.h>
#include <Lumino/UI/UILayoutPanel.h>
#include "LayoutImpl.h"

LN_NAMESPACE_BEGIN

//==============================================================================
// UIControl
//==============================================================================
LN_UI_TYPEINFO_IMPLEMENT(UIControl, UIElement);
LN_TR_PROPERTY_IMPLEMENT(UIControl, HAlignment, HContentAlignment, tr::PropertyMetadata());
LN_TR_PROPERTY_IMPLEMENT(UIControl, VAlignment, VContentAlignment, tr::PropertyMetadata());

static const String CommonStates = _T("CommonStates");
static const String FocusStates = _T("FocusStates");
static const String ValidationStates = _T("ValidationStates");
const String UIControl::NormalState = _T("Normal");
const String UIControl::MouseOverState = _T("MouseOver");
const String UIControl::PressedState = _T("Pressed");
const String UIControl::DisabledState = _T("Disabled");
const String UIControl::UnfocusedState = _T("Unfocused");
const String UIControl::FocusedState = _T("Focused");
const String UIControl::ValidState = _T("Valid");
const String UIControl::InvalidState = _T("Invalid");

//------------------------------------------------------------------------------
UIControl::UIControl()
	//: m_visualTreeRoot(nullptr)
{
}

//------------------------------------------------------------------------------
UIControl::~UIControl()
{
	//LN_SAFE_RELEASE(m_visualTreeRoot);
}

//------------------------------------------------------------------------------
void UIControl::Initialize(detail::UIManager* manager)
{
	UIElement::Initialize(manager);
	auto* vsm = GetVisualStateManager();
	vsm->RegisterVisualState(CommonStates, NormalState);
	vsm->RegisterVisualState(CommonStates, MouseOverState);
	vsm->RegisterVisualState(CommonStates, PressedState);
	vsm->RegisterVisualState(CommonStates, DisabledState);
	vsm->RegisterVisualState(FocusStates, UnfocusedState);
	vsm->RegisterVisualState(FocusStates, FocusedState);
	GoToVisualState(NormalState);


	m_items = RefPtr<UIElementCollection>::MakeRef(this);
	auto panel = NewObject<UIAbsoluteLayout>(manager);
	SetLayoutPanel(panel);
}

//------------------------------------------------------------------------------
bool UIControl::IsFocusable() const
{
	return true;
}

//------------------------------------------------------------------------------
UIElementCollection* UIControl::GetItems() const
{
	return m_items;
}

//------------------------------------------------------------------------------
void UIControl::AddChild(UIElement* element)
{
	m_items->Add(element);
}

//------------------------------------------------------------------------------
void UIControl::RemoveChild(UIElement* element)
{
	m_items->Remove(element);
}

//------------------------------------------------------------------------------
void UIControl::SetLayoutPanel(UILayoutPanel* panel)
{
	UILayoutPanel* oldPanel = m_itemsHostPanel;
	UILayoutPanel* newPanel = panel;

	// 既に持っていれば取り除いておく
	if (m_itemsHostPanel != nullptr && m_itemsHostPanel != panel)
	{
		RemoveVisualChild(m_itemsHostPanel);
		m_itemsHostPanel = nullptr;
	}

	// 新しく保持する
	if (panel != nullptr)
	{
		AddVisualChild(panel);
		m_itemsHostPanel = panel;
	}

	// 変更通知
	if (oldPanel != newPanel)
	{
		OnLayoutPanelChanged(newPanel);
	}
}

//------------------------------------------------------------------------------
UILayoutPanel* UIControl::GetLayoutPanel() const
{
	return m_itemsHostPanel;
}


////------------------------------------------------------------------------------
//int UIControl::GetVisualChildrenCount() const
//{
//	return (m_visualTreeRoot != nullptr) ? 1 : 0;
//}
//
////------------------------------------------------------------------------------
//ILayoutElement* UIControl::GetVisualChild(int index) const
//{
//	LN_THROW(0 <= index && index < GetVisualChildrenCount(), OutOfRangeException);
//	return m_visualTreeRoot;
//}
//
//------------------------------------------------------------------------------
Size UIControl::MeasureOverride(const Size& constraint)
{
#if 1
	Size desiredSize = UIElement::MeasureOverride(constraint);

	m_itemsHostPanel->MeasureLayout(constraint);
	const Size& childDesiredSize = m_itemsHostPanel->GetLayoutDesiredSize();

	desiredSize.width = std::max(desiredSize.width, childDesiredSize.width);
	desiredSize.height = std::max(desiredSize.height, childDesiredSize.height);

	return desiredSize;
#else
	return detail::LayoutImpl<UIControl>::UILayoutPanel_MeasureOverride(
		this, constraint,
		[](UIControl* panel, const Size& constraint) { return panel->UIElement::MeasureOverride(constraint); });
	//Size desiredSize = UIElement::MeasureOverride(constraint);
	//if (m_visualTreeRoot != nullptr)
	//{
	//    m_visualTreeRoot->MeasureLayout(constraint);
	//    const Size& childDesiredSize = m_visualTreeRoot->GetDesiredSize();

	//    desiredSize.width = std::max(desiredSize.width, childDesiredSize.width);
	//    desiredSize.height = std::max(desiredSize.height, childDesiredSize.height);
	//}
	//return desiredSize;
#endif

}

//------------------------------------------------------------------------------
Size UIControl::ArrangeOverride(const Size& finalSize)
{
#if 1
	Size childDesiredSize = m_itemsHostPanel->GetLayoutDesiredSize();
	childDesiredSize.width = std::max(finalSize.width, childDesiredSize.width);
	childDesiredSize.height = std::max(finalSize.height, childDesiredSize.height);
	m_itemsHostPanel->ArrangeLayout(RectF(0.0f, 0.0f, childDesiredSize));
	return finalSize;
#else
	return detail::LayoutImpl<UIControl>::UILayoutPanel_ArrangeOverride(this, Vector2::Zero, finalSize);
	//RectF childFinal(0, 0, finalSize);
	//if (m_visualTreeRoot != nullptr)
	//{
	//    Size childDesiredSize = m_visualTreeRoot->GetDesiredSize();
	//    childDesiredSize.width = std::max(finalSize.width, childDesiredSize.width);
	//    childDesiredSize.height = std::max(finalSize.height, childDesiredSize.height);
	//    m_visualTreeRoot->ArrangeLayout(RectF(0, 0, childDesiredSize));
	//}
	//return finalSize;
#endif
}


//------------------------------------------------------------------------------
const HAlignment* UIControl::GetPriorityContentHAlignment()
{
	if (HContentAlignment.Get() == VAlignment::Stretch) return nullptr;
	return &HContentAlignment.Get();
}
//------------------------------------------------------------------------------
const VAlignment* UIControl::GetPriorityContentVAlignment()
{
	if (VContentAlignment.Get() == VAlignment::Stretch) return nullptr;
	return &VContentAlignment.Get();
}

//------------------------------------------------------------------------------
void UIControl::OnRoutedEvent(const UIEventInfo* ev, UIEventArgs* e)
{
	// TODO: ここでやるべきではない。MFC なら PreTranslate 相当なので。On～で行う。
	if (ev == UIElement::MouseEnterEvent)
	{
		GoToVisualState(MouseOverState);
	}
	else if (ev == UIElement::MouseLeaveEvent)
	{
		GoToVisualState(NormalState);
	}

	UIElement::OnRoutedEvent(ev, e);
}

//------------------------------------------------------------------------------
void UIControl::OnGotFocus(UIEventArgs* e)
{
	GoToVisualState(FocusedState);
}

//------------------------------------------------------------------------------
void UIControl::OnLostFocus(UIEventArgs* e)
{
	GoToVisualState(UnfocusedState);
}


//------------------------------------------------------------------------------
//void UIControl::SetVisualTreeRoot(UIElement* element)
//{
//	if (m_visualTreeRoot != nullptr)
//	{
//		m_visualTreeRoot->SetParent(nullptr);
//	}
//
//	LN_REFOBJ_SET(m_visualTreeRoot, element);
//
//	if (m_visualTreeRoot != nullptr)
//	{
//		m_visualTreeRoot->SetParent(this);
//	}
//}

//------------------------------------------------------------------------------
void UIControl::OnLayoutPanelChanged(UILayoutPanel* newPanel)
{
}

//------------------------------------------------------------------------------
void UIControl::OnChildCollectionChanged(const tr::ChildCollectionChangedArgs& e)
{
	switch (e.action)
	{
	case tr::NotifyCollectionChangedAction::Add:
		if (LN_CHECK_STATE(e.newItems.GetCount() == 1)) return;	// TODO
		m_itemsHostPanel->GetChildren()->Insert(e.newStartingIndex, e.newItems.GetAt(0));
		break;
	case tr::NotifyCollectionChangedAction::Move:
		LN_NOTIMPLEMENTED();
		break;
	case tr::NotifyCollectionChangedAction::Remove:
		m_itemsHostPanel->GetChildren()->RemoveAt(e.oldStartingIndex);
		break;
	case tr::NotifyCollectionChangedAction::Replace:
		LN_NOTIMPLEMENTED();
		break;
	case tr::NotifyCollectionChangedAction::Reset:
		m_itemsHostPanel->GetChildren()->Clear();
		break;
	default:
		break;
	}
}

LN_NAMESPACE_END
