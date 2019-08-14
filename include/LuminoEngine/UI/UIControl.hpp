﻿#pragma once
#include "UIElement.hpp"

namespace ln {
class UILayoutPanel;

// TODO: TextBox の TextArea みたいに、UIElement の派生ではなくて、内部的なパーツとして利用する ScrollArea みたいなのを作ってみたらどうだろう。
// → 2019/8/12 作業中。いったん ScrollViewer を継承から外した
/*
    Note:

ScrollView の位置づけ
    ・シンプルなコントロールにスクロールバー分のメモリは使いたくないが・・・

Control
    ・Contet だろうが Items だろうが、フォーカスを持ったりタブストップしたり、共通はある。

直近の問題
    ・ListView に addd したい。addChild と addItem どっち使えばいいの？
    ・ListView は removeAllChildren じゃなくて removeAllItems の方がいいよね。
    ・ListView からは removeAllChildren は見えないようにしたい。逆もしかり。

LeafControl, ContentsControl, CollectionControl を分けようぜ、という方針。
Leaf がベースでいいかな。これは普通の UIControl。


    アイコンの標準プロパティ化について
    ----------
    やっぱりやっておきたい。
    必要なのは Content を持つ各種コントロール。
    HeaderdItemsControl
    HeaderdContentContorl なんかもあてはまる。
    子要素の配置領域を UIContentsArea みたいにクラス化しておいて、このクラスは領域の上下左右にアイコンなどビジュアル要素を任意に足せるようにしてみる。
    で、各種 Control はこれをヘルパーとして使う。
    こんな方針でいいかな。
    →アイコンだけでなく、コンテンツ周辺のコントロール追加全体的に使用したい。例えば、テキストボックスの右側にボタン追加したり。

    ### 任意要素の追加を提供するべきか？
    直近ではボタンを追加したい。
    その方法だけど…
    ・google 検索ページのinputには、音声入力ボタンと検索ボタン、2つが横に並んでいる。
    ・NumUpDown は上下に並ぶ２つのボタンで実装される。
    ・パスワードinputとかは、pushbutton ではなくToggleButton を使うこともある。
    ・ボタンクリックでコンテキストメニューを出すようなカスタムボタンを配置したいこともある。プロパティグリッドのドロップダウンとか。
    ・VisualStudio のソリューションエクスプローラ検索欄では、ドロップダウンのテキストボックスの中に検索ボタンがある
    ・ECサイトとか、検索バーの左側にカテゴリのドロップダウン付けたり
    ContainerItem だと、
    ・シーンのアウトラインのツリービューで、アイテムの右側に[表示/非表示]と[編集ロック]ボタンを置いたり。

    正しくやる方法 (ContentControl なら、addChild で Layout と Icon と TextBlock 追加したり、LeafControl なら measureOverride) は
    それはそれでいいんだけど、ただアイコン追加するだけのためにそれをやるのはめんどくさすぎる。

    ただ、setIcon() みたいな関数は公開しないほうがいいと思う。
    Window はタイトルバーへのアイコン設定関数を公開するが、それと混乱する。

    UE4はaddChild で Layout と Icon と TextBlockで作るらしい。
    https://www.reddit.com/r/unrealengine/comments/4f1z8v/how_can_i_make_a_button_with_both_text_and_a_icon/

    検索ボックスの実装。
    UnrealEngine\Engine\Source\Runtime\Slate\Private\Widgets\Input\SSearchBox.cpp
    SEditableTextBox が持っている TSharedPtr<SHorizontalBox> Box; に slot を差し込む形でボタンを配置している。

    …やっぱりvisualは自分でlayoutつかってレイアウトして、ContentPresenter とか使って子要素配置してもらうのが正しいのか・・・。

    JavaFX では、TextField を枠無しにして、ボタンと並べるみたい。
    https://lawrencepremkumar.wordpress.com/2012/09/15/custom-textfield-in-javafx-2-2/
*/
class UIControl
	: public UIElement
{
public:
    UIControl();
	void init();
    virtual void onDispose(bool explicitDisposing) override;



    /** コンテンツの横方向の配置方法を設定します。 */
    void setHorizontalContentAlignment(HAlignment value);

    /** コンテンツの横方向の配置方法を取得します。 */
    HAlignment horizontalContentAlignment() const;

    /** コンテンツの縦方向の配置方法を設定します。 */
    void setVerticalContentAlignment(VAlignment value);

    /** コンテンツの縦方向の配置方法を取得します。 */
    VAlignment verticalContentAlignment() const;


	void addElement(UIElement* element);
    void removeElement(UIElement* element);

	void removeAllChildren();


	///**
	//	子要素をレイアウトするための UILayoutPanel を設定します。

	//	デフォルトは nullptr です。
	//*/
	//void setLayoutPanel(UILayoutPanel* panel);

	//UILayoutPanel* layoutPanel() const;

	const List<Ref<UIElement>>& logicalChildren() const { return m_logicalChildren; }

protected:
	///** この要素内の子ビジュアル要素の数を取得します。 */
	//virtual int getVisualChildrenCount() const;

	///** 子ビジュアル要素を取得します。奥にある要素が先、手前にある要素が後になります。(Zオーダーやアクティブ状態の考慮は実装側で行うこと) */
	//virtual UIElement* getVisualChild(int index) const;

	virtual Size measureOverride(const Size& constraint) override;
	virtual Size arrangeOverride(const Size& finalSize) override;

    virtual void onLayoutPanelChanged(UILayoutPanel* newPanel);

    bool m_enabledDirectChildrenContentAlignment;

    List<Ref<UIElement>> m_logicalChildren;
private:

    //Ref<UILayoutPanel> m_layout;
	//Size m_layoutDesiredSize;	// Layout is state-less
};



} // namespace ln

