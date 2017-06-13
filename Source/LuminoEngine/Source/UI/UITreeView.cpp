
#include "Internal.h"
#include <Lumino/UI/UITextBlock.h>
#include <Lumino/UI/UITreeView.h>
#include <Lumino/UI/UILayoutPanel.h>
#include "UIManager.h"

LN_NAMESPACE_BEGIN
namespace tr
{

//==============================================================================
// UITreeViewItem
//==============================================================================
	LN_TR_REFLECTION_TYPEINFO_IMPLEMENT(UITreeViewItem, UIControl)

//------------------------------------------------------------------------------
UITreeViewItem::UITreeViewItem()
	: m_header(nullptr)
{
}

//------------------------------------------------------------------------------
UITreeViewItem::~UITreeViewItem()
{
}

//------------------------------------------------------------------------------
void UITreeViewItem::initialize()
{
	UIControl::initialize();
	SetHContentAlignment(HAlignment::Left);
	SetHAlignment(HAlignment::Stretch);
	GoToVisualState(UIVisualStates::NormalState);
}

//------------------------------------------------------------------------------
void UITreeViewItem::SetHeader(UIElement* header)
{
	RemoveVisualChild(m_header);

	m_header = header;
	m_header->SetBackground(Brush::Black);
	m_header->SetHeight(16);	// TODO:

	if (m_header != nullptr)
		AddVisualChild(m_header);
}

//------------------------------------------------------------------------------
Size UITreeViewItem::measureOverride(const Size& constraint)
{
	Size desiredSize(16, 0);	// TODO: Branch の余白は後で考える http://doc.qt.io/qt-4.8/stylesheet-examples.html#customizing-qtreeview

	// measure Header
	m_header->measureLayout(constraint);
	Size headerSize = m_header->getDesiredSize();

	// measure Items
	UILayoutPanel* itemsPanel = GetLayoutPanel();
	itemsPanel->measureLayout(constraint);
	Size panelSize = itemsPanel->getDesiredSize();

	// 下方向に結合する
	desiredSize.height += headerSize.height;
	desiredSize.height += panelSize.height;
	desiredSize.width = std::max(headerSize.width, panelSize.width);

	Size thisSize = ln::detail::LayoutHelper::MeasureElement(this, constraint);

	return Size::max(desiredSize, thisSize);

	// ※GetLayoutPanel() で得られる UILayoutPanel の measure をここで行うので 
}

//------------------------------------------------------------------------------
Size UITreeViewItem::arrangeOverride(const Size& finalSize)
{
	Size expanderSize(16, 16);	// TODO: Branch の余白は後で考える

	// Header
	Size headerSize = m_header->getDesiredSize();
	Rect headerRect(expanderSize.width, 0, finalSize.width - expanderSize.width, std::max(expanderSize.height, headerSize.height));
	m_header->arrangeLayout(headerRect);
	
	// Items
	Rect itemsRect(expanderSize.width, headerRect.height, finalSize.width - expanderSize.width, finalSize.height - headerRect.height);
	GetLayoutPanel()->arrangeLayout(itemsRect);

	return finalSize;
}


//==============================================================================
// UITreeView
//==============================================================================
LN_TR_REFLECTION_TYPEINFO_IMPLEMENT(UITreeView, UIControl)

//------------------------------------------------------------------------------
UITreeViewPtr UITreeView::create()
{
	auto ptr = UITreeViewPtr::makeRef();
	ptr->initialize();
	return ptr;
}

//------------------------------------------------------------------------------
UITreeView::UITreeView()
{
}

//------------------------------------------------------------------------------
UITreeView::~UITreeView()
{
}

//------------------------------------------------------------------------------
void UITreeView::initialize()
{
	UIControl::initialize();
	SetHContentAlignment(HAlignment::Stretch);

	auto panel = RefPtr<UIStackPanel>::makeRef();
	panel->initialize();
	panel->SetHAlignment(HAlignment::Stretch);
	panel->SetVAlignment(VAlignment::Stretch);
	SetLayoutPanel(panel);
	GoToVisualState(UIVisualStates::NormalState);
}

////------------------------------------------------------------------------------
//UITreeViewItemPtr UITreeView::AddTextItem(const String& text)
//{
//	auto textBlock = RefPtr<UITextBlock>::MakeRef();
//	textBlock->initialize(getManager());
//	textBlock->SetText(text);
//	return AddItem(textBlock);
//}
//
////------------------------------------------------------------------------------
//UITreeViewItemPtr UITreeView::AddItem(UIElement* item)
//{
//
//	// 受け取った item を UITreeViewItem でラップして、UITreeViewItem をリストに入れる
//	auto listItem = RefPtr<UITreeViewItem>::MakeRef();
//	listItem->initialize(getManager());
//	listItem->SetContent(item);
//	GetItems()->Add(listItem);
//	return listItem;
//}

} // namespace tr
LN_NAMESPACE_END
