﻿
#include "Internal.hpp"
#include <LuminoEngine/UI/UIStyle.hpp>
#include <LuminoEngine/UI/UILayoutPanel.hpp>
#include <LuminoEngine/UI/UIButton.hpp>
#include <LuminoEngine/UI/UITextBlock.hpp>
#include <LuminoEngine/UI/UITreeView.hpp>

namespace ln {

//==============================================================================
// UITreeItem

UITreeItem::UITreeItem()
    : m_ownerTreeView(nullptr)
{
}

void UITreeItem::init()
{
    UICollectionItem::init();

    auto vsm = getVisualStateManager();
    vsm->registerState(UIVisualStates::CommonStates, UIVisualStates::Normal);
    vsm->registerState(UIVisualStates::CommonStates, UIVisualStates::MouseOver);
    vsm->registerState(UIVisualStates::CommonStates, UIVisualStates::Pressed);

    // TODO: CreationContext とか用意したほうがいいかも。init を public にしないとダメだし。
	m_expanderButton = makeObject<UIToggleButton>(UICreationContext::DisabledAutoAddToPrimaryElement);
    //m_expanderButton = makeRef<UIToggleButton>();
    //m_expanderButton->m_objectManagementFlags.unset(detail::ObjectManagementFlags::AutoAddToPrimaryElement);
    //m_expanderButton->init();

	m_expanderButton->addClass(u"UITreeItem-Expander");
    m_expanderButton->connectOnChecked(bind(this, &UITreeItem::expander_Checked));
    m_expanderButton->connectOnUnchecked(bind(this, &UITreeItem::expander_Unchecked));

    //m_expanderButton->setBackgroundColor(Color::Red);

	addVisualChild(m_expanderButton);

    //auto layout = makeObject<UIStackLayout_Obsolete>();
    auto layout = makeObject<UIStackLayout>();
    layout->setOrientation(Orientation::Vertical);
    m_itemsLayout = layout;
    addVisualChild(m_itemsLayout);
}

void UITreeItem::setContent(UIElement* value)
{
    assert(!m_headerContent);    // TODO
    m_headerContent = value;
    addVisualChild(m_headerContent);
}

void UITreeItem::addChild(UITreeItem* item)
{
    if (LN_REQUIRE(item)) return;
    m_items.add(item);
    item->m_ownerTreeView = m_ownerTreeView;
    m_itemsLayout->addVisualChild(item);   // TODO: 多重追加対策
}

void UITreeItem::onExpanded()
{
    m_ownerTreeView->makeChildItems(this);
    
    for (auto& child : m_items) {
        child->m_internalVisibility = UIVisibility::Visible;
    }
}

void UITreeItem::onCollapsed()
{
    for (auto& child : m_items) {
        child->m_internalVisibility = UIVisibility::Collapsed;
    }
}

void UITreeItem::onClick(UIMouseEventArgs* e)
{
    UICollectionItem::onClick(e);
    m_ownerTreeView->onItemClick(this, e);
}

void UITreeItem::onViewModelChanged(UIViewModel* newViewModel, UIViewModel* oldViewModel)
{
	if (oldViewModel) {
		LN_NOTIMPLEMENTED();
	}

	m_model = dynamic_cast<UICollectionItemViewModel*>(newViewModel);
	if (!m_model) {
		LN_NOTIMPLEMENTED();
	}

    // TODO: model の変更通知対応
    if (m_model->getChildrenCount() > 0) {
        m_expanderButton->m_internalVisibility = UIVisibility::Visible;
    }
    else {
        m_expanderButton->m_internalVisibility = UIVisibility::Hidden;
    }

    UIElement::setContent(m_model->getValue(u""));
}

Size UITreeItem::measureOverride(UILayoutContext* layoutContext, const Size& constraint)
{
    //bool expanderVisible = false;
    //if (m_model) {
    //    //m_model
    //}


    struct ElementList : public IUIElementList {
        List<Ref<UITreeItem>>* list;
        virtual int getElementCount() const { return list->size(); }
        virtual UIElement* getElement(int i) const { return list->at(i); }
    } list;
    list.list = &m_items;

    Size size;

    // expander
    m_expanderButton->measureLayout(layoutContext, constraint);
    size.width += m_expanderButton->desiredSize().width;
    size.height = std::max(size.height, m_expanderButton->desiredSize().height);

    // header
    if (m_headerContent) {
        m_headerContent->measureLayout(layoutContext, constraint);
        size.width += m_headerContent->desiredSize().width;
        size.height = std::max(size.height, m_headerContent->desiredSize().height);
    }

    // children
    m_itemsLayout->measureLayout(layoutContext, constraint);
    size.width = std::max(size.width, m_itemsLayout->desiredSize().width);
    size.height += m_itemsLayout->desiredSize().height;

    Size desiredSize = UICollectionItem::measureOverride(layoutContext, constraint);

    return Size::max(size, desiredSize);
}

Size UITreeItem::arrangeOverride(UILayoutContext* layoutContext, const Size& finalSize)
{
    struct ElementList : public IUIElementList {
        List<Ref<UITreeItem>>* list;
        virtual int getElementCount() const { return list->size(); }
        virtual UIElement* getElement(int i) const { return list->at(i); }
    } list;
    list.list = &m_items;


    // expander
    float headerHeight = m_finalStyle->minHeight;
    Rect expanderSlot(0, 0, headerHeight, headerHeight);
    m_expanderButton->arrangeLayout(layoutContext, expanderSlot); // TODO: actualsize 返すようにしていいかも

    Rect area = Rect(expanderSlot.width, 0, finalSize.width - expanderSlot.width, expanderSlot.height);

    // header
	float headerContentHeight = 0;
    if (m_headerContent) {
        //area.x += m_expanderButton->m_actualSize.width;
        //area.width -= m_expanderButton->m_actualSize.width;

        Rect contentSlotRect;
        detail::LayoutHelper::adjustAlignment(
            area, m_headerContent->desiredSize(),
            m_ownerTreeView->m_finalStyle->horizontalContentAlignment, m_ownerTreeView->m_finalStyle->verticalContentAlignment, &contentSlotRect);


        m_headerContent->arrangeLayout(layoutContext, contentSlotRect);
		headerContentHeight = m_headerContent->actualSize().height;
    }

    // children
    area.y = std::max(m_expanderButton->actualSize().height, headerContentHeight);
    m_itemsLayout->arrangeLayout(layoutContext, area);

    return finalSize;
}

void UITreeItem::onRoutedEvent(UIEventArgs* e)
{
    if (e->type() == UIEvents::MouseDownEvent) {
        if (static_cast<UIMouseEventArgs*>(e)->getClickCount() == 2) {
        }
    }
    UICollectionItem::onRoutedEvent(e);
}

void UITreeItem::traverse(IUITreeItemVisitor* visitor)
{
    if (isRenderVisible()) {
        visitor->visit(this);
        for (auto& child : m_items) {
            child->traverse(visitor);
        }
    }
}

void UITreeItem::expander_Checked(UIEventArgs* e)
{
    onExpanded();
}

void UITreeItem::expander_Unchecked(UIEventArgs* e)
{
    onCollapsed();
}

//==============================================================================
// UITreeView
//  UITreeView : View-Model パターンで実装するときに使う
//  UITreeControl : 直接 Item を操作するのに使う
//  としていたが、UITreeControl は廃止する。やっぱり似たような名前の要素を増やしたくない。
//  代わりに UITreeView で両方対応する。
//  setModel() されたかどうかで、Virtialize モードかどうかを区別する。
//  Virtialize モードON の時は addItem() 禁止とかにする。（末尾に追加とかでもよさそうだけど）

UITreeView::UITreeView()
{
}

void UITreeView::init()
{
    UIItemsControl::init();

    auto layout = makeObject<UIStackLayout>();
    layout->setOrientation(Orientation::Vertical);
    setItemsLayoutPanel(layout);

    setHorizontalContentAlignment(HAlignment::Left);
    setVerticalContentAlignment(VAlignment::Center);
}

//void UITreeView::setModel(UICollectionModel* model)
//{
//    m_model = model;
//    //int count = m_model->getRowCount(nullptr);
//    //for (int i = 0; i < count; i++) {
//    //    auto itemModel = m_model->getIndex(i, 0, nullptr);
//    //    auto itemData = m_model->getData(itemModel, u"");
//
//    //    auto text = makeObject<UITextBlock>();
//    //    text->setText(itemData);
//
//    //    auto item = makeObject<UITreeItem>();
//    //    item->setContent(text);
//    //    item->setData(makeVariant(itemModel));
//
//    //    //if (!parent) {
//    //        addElement(item);
//    //    //}
//    //    //else {
//    //   //     parent->addChild(item);
//    //    //}
//    //}
//}

void UITreeView::onItemClick(UITreeItem* item, UIMouseEventArgs* e)
{
}

Ref<UITreeItem> UITreeView::onRenderItem(UICollectionItemViewModel* viewModel)
{
    auto item = makeObject<UITreeItem>();
    item->setViewModel(viewModel);
    return item;
}

void UITreeView::onViewModelChanged(UIViewModel* newViewModel, UIViewModel* oldViewModel)
{
	if (oldViewModel) {
		LN_NOTIMPLEMENTED();
	}

	m_model = dynamic_cast<UICollectionModel*>(newViewModel);
	if (!m_model) {
		LN_NOTIMPLEMENTED();
	}

	makeChildItems(nullptr);
}

void UITreeView::onSourcePropertyChanged(UINotifyPropertyChangedEventArgs* e)
{
    if (e->action() == UICollectionChangedAction::Reset) {
        removeAllChildren();
        makeChildItems(nullptr);
    }
}

Size UITreeView::measureOverride(UILayoutContext* layoutContext, const Size& constraint)
{
#if 0
    if (m_layoutMode == UITreeViewLayoutMode::IndentedList) {
        class LocalVisitor : public IUITreeItemVisitor
        {
        public:
            List<UITreeItem*>* list;
            virtual void visit(UITreeItem* item)
            {
                list->add(item);
            }
        };
        LocalVisitor v;
        v.list = &m_listLayoutCache;
        m_listLayoutCache.clear();

        for (auto& child : *m_logicalChildren) {
            static_pointer_cast<UITreeItem>(child)->traverse(&v);
        }

        if (m_listLayoutCache.size() >= 1) {
            for (auto& item : m_listLayoutCache) {
                item->measureLayout(layoutContext, constraint);
            }

            Size size = m_listLayoutCache.front()->desiredSize();

            // 全 item 固定サイズで算出
            size.height = size.height * m_listLayoutCache.size();

            return size;
        }
    }
#endif

    return UIItemsControl::measureOverride(layoutContext, constraint);
}

Size UITreeView::arrangeOverride(UILayoutContext* layoutContext, const Size& finalSize)
{
#if 0
    if (m_layoutMode == UITreeViewLayoutMode::IndentedList) {
        if (m_listLayoutCache.size() >= 1) {
            Rect rect;
            for (auto& item : m_listLayoutCache) {
                rect.width = item->desiredSize().width;
                rect.height = item->desiredSize().height;
                item->arrangeLayout(layoutContext, rect);
                rect.y += rect.height;
            }

            return finalSize;
        }
    }
#endif

    return UIItemsControl::arrangeOverride(layoutContext, finalSize);
}

void UITreeView::addItemInternal(UITreeItem* item)
{
    assert(item);
    item->m_ownerTreeView = this;
    addItem(item);
}

void UITreeView::makeChildItems(UITreeItem* item)
{
    assert(isVirtualize());

    UICollectionItemViewModel* itemModel = nullptr; // null is root
    if (item) {

        itemModel = item->m_model;
        int count = itemModel->getChildrenCount();
        if (item->m_items.size() != count) {
            item->removeAllChildren();

            for (int i = 0; i < count; i++) {
                auto childModel = itemModel->getItem(i);
                //auto itemData = m_model->getData(childModel, u"");

                //auto text = makeObject<UITextBlock>();
                //text->setText(itemData);

                //auto child = makeObject<UITreeItem>();
                //child->setContent(text);
                //child->setViewModel(childModel);
                //child->setData(makeVariant(childModel));
                auto child = onRenderItem(childModel);

                item->addChild(child);
            }
        }


    }
    else {
        int count = m_model->getItemCount();
        for (int i = 0; i < count; i++) {
            auto childModel = m_model->getItem(i);
            auto child = onRenderItem(childModel);

            addItemInternal(child);
        }
    }
}

void UITreeView::onItemAdded(UICollectionItem* item)
{
    //if (m_layoutMode == UITreeViewLayoutMode::IndentedList) {
    //    addVisualChild(item);
    //}
}

} // namespace ln

