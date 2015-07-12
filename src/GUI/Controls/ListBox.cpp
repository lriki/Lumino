/*
	・WPF
		ListBox
			Border	枠及び背景
				ScrollViewer
					Grid
						Rectangle	装飾
						ScrollContentPresenter
							ItemsPresenter
								VirtualizingStackPanel
									ListBoxItem
									ListBoxItem
									ListBoxItem
										Border
											ContentPresenter
												<いろいろ>
						ScrollBar	縦
						ScrollBar	横

*/
#include "../../Internal.h"
#include <Lumino/GUI/GUIManager.h>
#include <Lumino/GUI/ControlTemplate.h>
#include <Lumino/GUI/Controls/ListBox.h>
#include <Lumino/GUI/Controls/StackPanel.h>

namespace Lumino
{
namespace GUI
{
//=============================================================================
// ListBoxItem
//=============================================================================
LN_CORE_OBJECT_TYPE_INFO_IMPL(ListBoxItem, ContentControl);
LN_UI_ELEMENT_SUBCLASS_IMPL(ListBoxItem);

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ListBoxItem::ListBoxItem(GUIManager* manager)
	: ContentControl(manager)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ListBoxItem::~ListBoxItem()
{
}

//=============================================================================
// ListBoxItemList
//=============================================================================
LN_CORE_OBJECT_TYPE_INFO_IMPL(ListBoxItemList, CoreObjectList<ListBoxItem>);

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBoxItemList::OnItemAdded(ListBoxItem* item)
{
	m_owner->OnListBoxItemAdded(item);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBoxItemList::OnItemRemoved(ListBoxItem* item)
{
	m_owner->OnListBoxItemRemoved(item);
}

//=============================================================================
// ListBoxChrome
//=============================================================================
LN_CORE_OBJECT_TYPE_INFO_IMPL(ListBoxChrome, Decorator);
LN_UI_ELEMENT_SUBCLASS_IMPL(ListBoxChrome);

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ListBoxChrome::ListBoxChrome(GUIManager* manager)
	: Decorator(manager)
	, m_frameWidth(8)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ListBoxChrome::~ListBoxChrome()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBoxChrome::OnApplyTemplate(CombinedLocalResource* localResource)
{
	m_frameBrush = static_cast<Graphics::TextureBrush*>(localResource->GetItem(_T("ListBoxNormalFrameBrush")));
	m_bgBrush = static_cast<Graphics::TextureBrush*>(localResource->GetItem(_T("ListBoxNormalBackgroundBrush")));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBoxChrome::OnRender()
{
	Graphics::Painter painter(m_manager->GetGraphicsManager());
	painter.SetProjection(Size(640, 480), 0, 1000);	// TODO

	RectF bgRect = m_finalGlobalRect;
	RectF rect = m_finalGlobalRect;

	if (!m_isMouseOver)
	{
		bgRect.Inflate(-m_frameWidth, -m_frameWidth);
		painter.SetBrush(m_bgBrush);
		painter.DrawRectangle(bgRect);
	}

	painter.SetBrush(m_frameBrush);
	painter.DrawFrameRectangle(rect, m_frameWidth);
}


//=============================================================================
// ListBox
//=============================================================================
LN_CORE_OBJECT_TYPE_INFO_IMPL(ListBox, Control);
LN_UI_ELEMENT_SUBCLASS_IMPL(ListBox);

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ListBox::ListBox(GUIManager* manager)
	: Control(manager)
	, m_listBoxItems(LN_NEW ListBoxItemList(this))
{
	//m_itemsPanel.Attach(LN_NEW StackPanel(manager));
	//AddVisualChild(m_itemsPanel);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ListBox::~ListBox()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBox::InsertItem(int index, UIElement* element)
{
	RefPtr<ListBoxItem> item(LN_NEW ListBoxItem(m_manager));
	item->SetContent(element);
	m_listBoxItems->Insert(index, item);
}
#if 0
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
UIElement* ListBox::CheckMouseHoverElement(const PointF& globalPt)
{
	if (m_itemsPanel != NULL) {	// 子要素を優先
		UIElement* e = m_itemsPanel->CheckMouseHoverElement(globalPt);
		if (e != NULL) { return e; }
	}
	return UIElement::CheckMouseHoverElement(globalPt);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBox::ApplyTemplateHierarchy(CombinedLocalResource* parent)
{
	Control::ApplyTemplateHierarchy(parent);
	if (m_itemsPanel != NULL) {
		m_itemsPanel->ApplyTemplateHierarchy(m_combinedLocalResource);	// 再帰的に更新する
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBox::MeasureLayout(const SizeF& availableSize)
{
	if (m_itemsPanel != NULL) {
		m_itemsPanel->MeasureLayout(availableSize);	// 特に枠とかないのでそのままのサイズを渡せる
	}
	Control::MeasureLayout(availableSize);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBox::ArrangeLayout(const RectF& finalLocalRect)
{
	if (m_itemsPanel != NULL) {
		m_itemsPanel->ArrangeLayout(finalLocalRect);	// 特に枠とかないのでそのままのサイズを渡せる
	}
	Control::ArrangeLayout(finalLocalRect);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool ListBox::OnEvent(EventType type, EventArgs* args)
{
	if (m_itemsPanel != NULL) {
		if (m_itemsPanel->OnEvent(type, args)) { return true; }
	}
	return Control::OnEvent(type, args);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBox::Render()
{
	Control::Render();
	if (m_itemsPanel != NULL) {
		m_itemsPanel->Render();
	}
}
#endif

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBox::OnListBoxItemAdded(ListBoxItem* item)
{
	m_itemsPanel->GetChildren()->Add(item);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ListBox::OnListBoxItemRemoved(ListBoxItem* item)
{
	LN_THROW(0, NotImplementedException);
	//m_itemsPanel->GetChildren()->Remove(item);
}

} // namespace GUI
} // namespace Lumino

