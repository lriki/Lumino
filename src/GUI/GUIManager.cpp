/*

	★キーワード
	・テンプレート
	・バインディング
	・ルーティング イベント	https://msdn.microsoft.com/ja-jp/library/ms742806.aspx

	[2015/7/10]
		・SetPropertyValue() のキーは Property のポインタにする？名前にする？
		・依存関係プロパティはポインタとして公開する？名前にする？

		ビハインドコードからアクセスする場合はポインタの方が当然高速。
		XML からアクセスするときは "要素名.プロパティ名" という文字列で検索する必要がある。
		キーをポインタにしておくと、まず文字列、次にポインタ、といった2回の検索が必要になる。
		
		とは言っても、2回の検索が必要になるのは初期化時だけ。
		むしろアニメーションとかはリアルタイムにアクセスする必要があるので、
		ポインタあるいはハッシュ値のような数値で検索できたほうが高速。


	[2015/7/8] 座標の直値指定は必要？
		WinForms の Location プロパティは必要かということ。
		WPF ベースなので Canvas.SetLeft(button, 10); とかが適当。
		Siv3D も絶対座標は持ってないでマージンを持っていた。


	[2015/7/8] ホントに VisualTree 必要？

		Arrange で作る「クライアント領域の外側」に「コントロール」を配置したいから必要。
		ユーザーに公開しないとしても、内部的には持っておいた方が良い。
		それは本ライブラリのGUIモジュール1.0の実装を断念した原因だったはず。
		

	[2015/7/7] RoutedCommand と RoutedEvent の違いについて

		RoutedCommand は <Button Command="ScrollBar.LineUp" /> のように使う。
		これは、ボタンクリックに「スクロールバーの↑移動」という意味を持たせているということ。
		これ以外にも、ウィンドウのクローズとかコピペとかいろいろある。

		RoutedEvent は単にイベントを通知するだけ。

	[2015/7/5] イベント処理メモ
		本ライブラリとしては、
		・On〜 は RaiseEvent() を呼び出す方向で統一する。
			これはパフォーマンスを考慮した対応。
			RaiseEvent() は要素が持つハンドラを全検索したりするため、処理にやや時間がかかる可能性がある。
			On〜 で処理済みをチェックし、不必要な場合は RaiseEvent() しないようにすることで
			無駄な処理を行わないようにねらう。
		
		また、現在コントロールがルーティングイベントをハンドリングする方法は2つあり、
		1つは AddHandler すること、もう一つは LN_REGISTER_ROUTED_EVENT_HANDLER すること。
		(WPF では前者を「動的ハンドラ」後者を「静的ハンドラ」と呼んでいるみたい)

		静的ハンドラは動的ハンドラが呼び出される前に呼び出され、e.Handled がマークされた場合
		後続の動的ハンドラ呼び出し及びルーティングは行われない。
		https://msdn.microsoft.com/ja-jp/library/ms597875%28v=vs.110%29.aspx


	[2015/7/5] イベント処理メモ
		・On〜 はルーティングイベントのハンドラ。
			例えば、RaiseEvent(MouseDown) すると、そこから OnMouseDown() が呼ばれる。
			OnMouseDown() が RaiseEvent(MouseDown) するのではない点に注意。
			OnMouseDown() から RaiseEvent(Click) のように他のイベントを発行するのはあり。

			ただ、WPF の中では統一されていないみたい。
			MouseDown は Raise→On のようになっているが、ButtonBase.OnClick は On→Raise。




	[2015/6/30] そもそも VisualTree なんて作る必要あるの？
		
		LogicalTree と分ける仕組みは必要。
		例えば、ウィンドウのシステムボタン。
		これはボタンだけではなくPanelと組み合わせてレイアウトを調節したいもの。
		LogicalTree と同じようなレイアウトの仕組みがあると便利。

		テーマの切り替えは CSS じゃダメなの？
		→ CSS でウィンドウのシステムボタンをカスタマイズできるの？
		→ ウィンドウのクライアント領域の背景と枠を消して、Glass だけにしたいときは？
			→ 背景ON/OFFプロパティが必要になりそう・・・



	[2015/6/30] Qt Quick のようなコントロールのカスタマイズは？
		
		Qt Quick は JavaScript ライクなマークアップで、
		Button {
			background : <子コントロール>
			content : <コンテンツ>
		}
		みたいなかんじでコントロールのカスタマイズができる。
		すべて、
		・background
		・content
		・foreground
		の3レイヤーでカスタマイズできれば少しシンプルになりそうだが・・・？
		
		↓

		親コンテナの Padding を考慮できない。
		BackgroundPadding とか専用のプロパティが必要になってしまう。


	[2015/6/9] AddChild
		これは論理ツリー構築用に見える。
		WPF の ContentControll の AddChild は、引数で受け取ったオブジェクトを Content プロパティに入れてるだけだった。
		↓
		それでいい。
		テンプレートで作ったツリーはこれまで Logical 全然関係ないと思ってたけど、
		VisualTree の要素に LogcalTreeHelper.GetParent() すると、VisualTree 上の親要素が取れる。


	[2015/6/9] デフォルトの Chrome の状態変化に何を使う？(TemplateBinding と VisualState と Trigger)
		.NET 界隈では見た目の表現にはできるだけ VisualState を使ったほうがいいよね的な感じになっている。
		※VisualState は Silverlight から WPF に輸入された、

		ただ、基本的に Stopryboard (アニメーション) でしかプロパティ変化させられないので、Trigger の併用も必要。

		TemplateBinding は状態の通知に限らずいろいろなものを Template のどこに置くかを決めるのにも使う。
		こっちでの対応を優先してみる。
		



	[2015/6/9] Binding と TemplateBinding
		この2つは似ているが、データソースとして扱う対象が違う。
		Binding は 論理要素の DataContext、
		TemplateBinding は論理要素自身をソースとする。

		次のようなマークアップは、1つ目のラベルは Button.DataContext が変われば
		それに追随して変わるが、2つ目のラベルは変わらず、常に Button.IsPressed を参照し続ける。

		<Button x:Name="_button1" Height="32">
			<Button.Template>
				<ControlTemplate TargetType="{x:Type Button}">
					<StackPanel Orientation="Horizontal">
						<Label Content="{Binding Name}" />
						<Label Content="{TemplateBinding IsPressed}" />
					</StackPanel>
				</ControlTemplate>
			</Button.Template>
		</Button>

	
	[2015/6/9] CEGUI のプロパティ
		Editbox::addEditboxProperties() から辿るとすごく参考になる。


	[2015/6/9] CEGUI の描画

		Button を例に見てみると・・・

		PushButton クラスは押下中状態を管理するだけ。描画を行う人は別にいる。
		描画を行うのは FalagardButton クラスで、WindowRenderer クラスのサブクラス。

		WindowRenderer は Widget ごとにクラスが定義されていて、他にも TabControlWindowRenderer や EditboxWindowRenderer がある。
		ちょうどこれが今やろうとしている Chrome に相当しそう。
		Widget と WindowRenderer は 1対1。


		描画はわりと複雑。FalagardButton::render() から以下のように潜っていく。
		FalagardButton::render()
			WidgetLookFeel::render()
				StateImagery::render()
					LayerSpecification::render()
						SectionSpecification::render()
							ImagerySection::render()

		Window の持つ LookFeel が切り替われば、WidgetLookFeel::render() が切り替わる仕組み。

		ImagerySection::render() は次の3つを順に描画している。これが CEGUI の最小描画単位？
		・FrameComponent	・・・ 枠
		・ImageryComponent	・・・ 背景
		・TextComponent		・・・ テキスト


		WindowRenderer はオーナーコントロールを直接参照し、その状態を使って StateImagery を検索、StateImagery::render() を呼び出す。
		例えば PushButton なら、isPushed() なら "pushed" 文字列をキーに検索を行う。

		デフォルトのテーマファイルは↓っぽい。
		datafiles\looknfeel\AlfiskoSkin.looknfeel
	
		<ImagerySection> で、Compnent の配置を決めて名前をつける。
		<StateImagery> で、ある状態のときにどの <ImagerySection> を表示するかを決める。
		よく見てみれば、↑の複雑な描画パスも納得できる。
	


	[2015/6/9] Unity GUI
		GUI コントロールのカスタマイズ
		http://docs.unity3d.com/ja/current/Manual/gui-Customization.html

		GUI スキン
		http://docs.unity3d.com/ja/current/Manual/class-GUISkin.html
	
		CCS を意識している。すごく単純。

	[2015/6/7] Chrome その�A
		WPF の WindowChrome は UIElement ではない。
		タイトルバーの高さや、ウィンドウ枠の幅を定義する、単なるデータクラス。
		Window クラスはセットされている WindowChrome からこの各種領域を取得し、タイトルバーのD&D移動や
		境界D&Dによるリサイズを行っている。

		Button の基本的なテンプレートはこんな感じ。
		<Button.Template>
			<ControlTemplate TargetType="{x:Type Button}">
				<Microsoft_Windows_Themes:ButtonChrome SnapsToDevicePixels="true" 
					x:Name="Chrome" Background="{TemplateBinding Background}" 
					BorderBrush="{TemplateBinding BorderBrush}" RenderDefaulted="{TemplateBinding IsDefaulted}" 
					RenderMouseOver="{TemplateBinding IsMouseOver}" RenderPressed="{TemplateBinding IsPressed}">
					<ContentPresenter Margin="{TemplateBinding Padding}"
						VerticalAlignment="{TemplateBinding VerticalContentAlignment}"
						HorizontalAlignment="{TemplateBinding HorizontalContentAlignment}"
						RecognizesAccessKey="True"
						SnapsToDevicePixels="{TemplateBinding SnapsToDevicePixels}"/>
				</Microsoft_Windows_Themes:ButtonChrome>
			</ControlTemplate>
		</Button.Template>

		Button は IsMouseOver や IsPressed を公開し、ButtonChrome はそれにバインドしている。
		つまり、VisualState とかここでは関係ない。
	


	[2015/6/7] Chrome の考え方
		ButtonChrome や WindowChrome は、「ある程度一般的なビジュアルを備えたビジュアルツリー構築ユーティリティ」である。
		
		ホントにゼロからカスタマイズしたいときは Chrome は使わず、Rectangle 等の Shape を駆使して自作する。
		
		しかし、ほとんどの場合はカスタマイズしたいと言ってもやりたいことはウィンドウ枠の画像を変えたいとか、
		そういう「デフォルトコントロールから少し変えたい」程度であるはず。
		それを支援するのがこの Chrome。

		例えば、自作せずにデフォルトの Chrome を使っている場合はウィンドウスキンの png を差し替えるだけで
		テーマ変更を達成することができる。


	[2015/5/31]
		そもそも VisualTree は何で必要なの？
		→ 一番最初の GUI 設計で失敗した、「ウィンドウのシステムボタン」がこれを使うとすごく自然に作れる。


		Template とか、リソースシステムの目的は？
		無いとダメなの？何ができなくなるの？
		→ ぶっちゃけ無くても何とかなる。
			実際に必要になるのはテーマ変更したいってなった時くらいかも。
			
			あと、デフォルトの VisualTree のレイアウトを変更したいとき。
			コモンコントロールを変更したいっていうのはほとんどないと思う。UIデザインの学習コスト的に。
			変更するとしたら、ゲームには共通であるけどタイトルごとにデザインの違う物。例えば装備スロットとかになるかなぁ…。
			もしそれをこの GUI モジュールのコモンコントロールとするなら意味を持ってくる。
			でも、これも最悪 VisualTree を直接いじればいいだけか・・・。







	[2015/5/31]
		対応するビハインドコードのある、いわゆる普通にUIパーツの配置に使う XML は、「レイアウト」という言い方をしてみる。
		x:Class 属性を持っていたりする。

		コントロールが生成される流れは、

		
		・new Window()
		・レイアウトから LogicalTree のインスタンスを作る。
			・子要素に <Button> とかあれば GUIManager::CreateUIElement("Button")
		・UpdateTemplate() を再帰的に実行していく。(VisualTree を作る)
			関数の引数には ResourceDictionary を渡す。




	[2015/5/31] VisualTree の UIElement であるかの区別

		・ControlTemplate から作ったものは VisualTree。


	[2015/5/29] テーマの動的変更
		
		もし完全な MVVM で実装がされているなら、View をルートから全て作り直してしまえば良い。
		・・・が、完全な MVVM は現実的ではない・・・。
		「VisualTreeを再構築する」という方向で考えていく。
		
		VisualTree の UIElement は全て Release() される。
		対して、LogicalTree の UIElement は残り続ける。
		もしテーマの動的変更をできるようにするなら、LogicalTree には描画に関係するオブジェクトを一切置かない。
		・・・ようにする必要は無いか。
		ブラシの色くらいだったら



	[2015/5/29] Template と言語バインダとユーザーコントロール

		C# 側で Control を継承したユーザーコントロールを作ったとする。
		それを XML 上に書くことはできる？

		→ C++ 側が知らない新しい型を登録するということになる。動的な型情報の追加。
		→ または、C++ 側で new したいときにクラス名を使ってファクトリ関数をコールバックする。
		   C# 側はサブクラスを new して Control の Handle を返すだけ。
		   Managed インスタンスはグローバル管理配列に入れておく。これだけでたぶんOK



	[2015/5/29] アニメーション

		Animation モジュールの内容は基本的に GUI 側でラップする (AnimationTimeline のサブクラス)
		これは XML からいろいろ定義できるようにするため。

		WPF のアニメーションはこんな感じ。
		   System.Windows.Media.Animation.AnimationTimeline
              System.Windows.Media.Animation.BooleanAnimationBase
              System.Windows.Media.Animation.ByteAnimationBase
              System.Windows.Media.Animation.CharAnimationBase
              System.Windows.Media.Animation.ColorAnimationBase
              System.Windows.Media.Animation.DecimalAnimationBase
              System.Windows.Media.Animation.DoubleAnimationBase
              System.Windows.Media.Animation.Int16AnimationBase
              System.Windows.Media.Animation.Int32AnimationBase
              System.Windows.Media.Animation.Int64AnimationBase
              System.Windows.Media.Animation.MatrixAnimationBase
              System.Windows.Media.Animation.ObjectAnimationBase
              System.Windows.Media.Animation.Point3DAnimationBase
              System.Windows.Media.Animation.PointAnimationBase
              System.Windows.Media.Animation.QuaternionAnimationBase
              System.Windows.Media.Animation.RectAnimationBase
              System.Windows.Media.Animation.Rotation3DAnimationBase
              System.Windows.Media.Animation.SingleAnimationBase
              System.Windows.Media.Animation.SizeAnimationBase
              System.Windows.Media.Animation.StringAnimationBase
              System.Windows.Media.Animation.ThicknessAnimationBase
              System.Windows.Media.Animation.Vector3DAnimationBase
              System.Windows.Media.Animation.VectorAnimationBase

		いまのところキーフレームアニメーションは考えなくて良いと思う。
		2点間で、傾きだけ指定できれば。

		AnimationTimeline は共有リソース。
		Storyboard も共有リソース。

		・Storyboard::Begin() で AnimationClock を作る。AnimationClock にはターゲット要素と AnimationTimeline をセット。
		・AnimationClock は GUIManager に登録。
		・GUIManager への InjectTime() で全ての AnimationClock を遷移させる。
		・AnimationClock は現在の値をターゲットにセットする。

		ちなみに WPF は AnimationClock がターゲットを持たないみたい。
		AnimationClock も共有できてメモリ使用量減るかもしれないけど、
		そこまで作りこむと複雑になりすぎる気がするのでとりあえず上記方法で進める。



	[2015/5/29] ネイティブなフローティングウィンドウの必要性
	
		・ウィンドウメニュードロップダウン
		・コンテキストメニュー
		・コンボボックスのドロップダウン
		・バルーン
		・IME
		・ドッキングウィンドウ
		
		例えばコンボボックスのドロップダウンは、Visual 的な親子関係は持っていない。
		
		ただ、複数ウィンドウ作ると「ホントのフルスクリーン」ができなくなる。
		

	[2015/5/28] RoutedEvent の必要性
		
		すぐ思いつく一番効果が高いものは ItemsControl の ItemClicked だと思う。
		例えばクリックしたときのハイライトをカスタマイズしたくてテンプレートを組んだとする。
		単純な ListBox であれば ListBoxItem がクリックされた時、直接の親コントロールである ListBox に
		イベントを通知すればよい。しかし、テンプレートを組むと複雑な VisualTree ができることになり、
		当然その VisualTree の中でもイベントを受け取りたい。さらに、それは Handled=false で
		親コントロールへ通知されていくべき。


	[2015/5/28] 普通のイベントと RoutedEvent

		<Window x:Class="WpfApplication1.MainWindow"
				xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
				xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
				Title="MainWindow" Height="350" Width="525">
			<StackPanel>
				<Button x:Name="_button1" Height="32"/>
				<Button x:Name="_button2" Height="32"/>
			</StackPanel>
		</Window>

		public partial class MainWindow : Window
		{
			public MainWindow()
			{
				InitializeComponent();
				this._button1.Click += new RoutedEventHandler(Button1_Click_CLR);
				this.AddHandler(Button.ClickEvent, new RoutedEventHandler(Button1_Click_Routed)); 
			}

			private void Button1_Click_CLR(object sender, RoutedEventArgs e)
			{
				Console.WriteLine("Button1_Click_CLR");
				//e.Handled = true;
			} 

			private void Button1_Click_Routed(object sender, RoutedEventArgs e)
			{
				Console.WriteLine("Button1_Click_Routed");
			} 
		}


		Button1_Click_CLR は、_button1 をクリックした瞬間に呼ばれる。
		その後、RoutedEvent によりさかのぼっていって Button1_Click_Routed が呼ばれる。
		もし Button1_Click_CLR で e.Handled = true; とか書くと Button1_Click_Routed は呼ばれなくなる。

		また、_button2 をクリックすると Button1_Click_Routed だけ呼ばれる。




	[2015/5/26] Property の必要性
		無い場合、マークアップパーサから SetValue("Width", 10) したとき等、
		プロパティ名に対する値の格納先メンバ変数を決める if なり switch なりを、
		サブクラスにすべて実装しなければならない。
		C++ だけならまだしも、各言語バインダでそれをやるのはちょっと実行時間的なコストがかかる…。
		なによりめんどくさすぎる。


	[2015/5/21] 描画機能

		・ビットマップ描画
			・ビットマップベースの枠描画 (RGSS ウィンドウスキン)
		・矩形

		・

	■UIElementTemplate(ControlTemplate) と、UIElementFactory(FrameworkElementFactory) の違い
		両方とも SetValue や Style、Trigger を設定することができ
		なんとなく似ているが、使用目的はぜんぜん違う。
		・UIElementFactory
			単純に1つの UIElement を作る。難しい制約とか無し。
			子要素を追加することで、ツリーそのものを表せる。
			インスタンス化したものはそのまま VisualツリーにもLogicalツリーにも使える。
		・UIElementTemplate
			1つの論理的なコントロールの作成に必要な情報の集合。例えば Button は Chrome や TextBlock から成り立っているが、
			使うときはそんなの気にしないで、ひとつの Button として Visual または Logical ツリーに繋ぐことができる。
			UIElementTemplate を階層構造にすることはできない。
			【対応する Presenter が必ず必要になる。】
	
	■UIElementFactoryからのインスタンス化
		UIElementFactory::createUIElement() で自身を作成。
		子要素 Factor があれば、それぞれの createUIElement() を呼び、
		作成された UIElement を階層構造に結合して返す。
		UIElement を作成するときは、名前によってManager から UIElementTemplate を取得し、
		UIElementTemplate.createUIElement() を呼び出す。
		1つの create 処理で UIElementTemplate と UIElementFactory が互い違いに呼び出されてかなり複雑になるので注意。
		【Manager にコントロールとして直接登録されるのは UIElementTemplate である】

・virtual 関数
	・BinderLib の関数をコールバックで呼び出す。(この呼び出される BinderLib の関数を Ploxy 関数と呼ぶ)
	・Ploxy 関数は static 関数であること。(C# の場合、メンバメソッドだとGC対象から外す等の処理が必要で面倒)
	・Ploxy 関数の型は LFTypedef.h に定義(typedef)する関数ポインタ型と同じであること。
	  つまり、RefObject を受け取る場合は APILib が握っている Handle を受け取ることになる。
	・Ploxy 関数は Handle から this にあたる Wrapper オブジェクトをグローバルな Manager から検索し、
	  目的のメンバメソッドを呼び出す。Ploxy 関数は virtual 関数と 1:1 で呼び出す。
	  つまり、virtual 関数の数だけ Ploxy 関数が定義されることになる。
	  
	・APILib は、公開する全ての RefObject クラスに対して、サブクラスを実装する。(このサブクラスを Ploxy クラスと呼ぶ)
	  APILib から公開される直接のオブジェクトは、この Ploxy である。
	・Ploxy クラスは、CoreLib クラスが公開する全ての virtual 関数をオーバーライドする。(スーパークラスも含めて)
	  例えば、Sprite は SceneNode を継承するが、このときサブクラス SpritePloxy は DrawSubset() を実装する。
	・Ploxy はメンバ変数に LFTypedef.h に定義(typedef)する関数ポインタ型のメンバ変数を持つ。
	  BinderLib 側に定義した Ploxy 関数のポインタは、ここに設定される。
	・オーバーライドの実装は、この関数ポインタを経由して Ploxy 関数を呼び出す。
	・公開 API 関数は、sprite->DrawSubset() のように関数を呼ぶのではなく、
	  sprite->Sprite::DrawSubset() のように、CoreLib のクラス名を指定して呼び出す。
	  こうしないと、無限再起呼び出しに陥る。
	  
	
・コールバック関数
	・基本方針は virtual 関数と同じく、Ploxy クラスを利用する方法を使う。
	
	・CoreLib に実装するコールバックの setter 等の引数型は Delegate クラスを使用してよい。
	  例えば、ArrayList::Sort(Delegate02<T, T> pred) や XmlParser::SetFoundElementCallback(Delegate01<XmlElement> callback) として定義し、
	  Ploxy クラスの関数をセットする。
	

	↑こんな実装にするのは、CoreLib 自体の使い勝手を落としたくないから。Delegate を使いたい。
	  Delegate が使えなくても、static 関数とユーザーポインタを登録することで実現はかのう。
	  ただし、Delegate が使えないとすると、event の管理がものすごく面倒なことになる。

	
	・テンプレート型のコールバックは？
	  型は RefObject を継承している制限にする。

・event

・メッセージキュー
・オブジェクト配列
・構造体配列
	・頂点配列、DrawLines 等。
	
	・個別定義がいいかも。PointList みたいに。(C# の PointCollection) とか。
		・この List は RefObject であるほうが自然な気がする。
			・そうすると、CoreLib 無いでも Point[] ではなく PointList クラスを作って持っておくべき。

	・頂点バッファのようなカスタム型構造体配列
		・SlimDX の VertexBuffer.Lock は DataStream というクラスを返す。
		  これの SetRange() に構造体配列を渡し、メモリコピーしている。
		・XNA は VertexBuffer.SetData() で構造体配列を渡す。
		・↑2つは構造体配列を前提にしているので Ruby とかに持っていくにはあまり向かないかも・・・。
		
		・RefObjectList の対になる StructList が必要かもしれない。
			・RefObjectList は参照カウントも操作するList。
			・StructList は実体コピーを行う List。
			  バインダでは型引数に指定された型のインスタンスを new して返す。
			
		・頂点バッファは特殊なリストにするしかないかも。
		  まじめにやるなら、バインダ側で set しようとしている配列の中身が全て LNote の構造体型であることと、
		  かつそのフィールドもすべて構造体型であることをチェックしなければならない。
		


-------------------------------------------------------------------------------
■ UIデザインパターン？

	MVC とか MVVM とか、特定のパターンは想定しない。
	タイトルによって実装パターンはかなり変わるはずなので、
	ネイティブなWinAPI とか gtk とか、ほんとに単純なウィンドウシステムとしてまとめる。
	

-------------------------------------------------------------------------------
■ そもそもXAMLなんて必要なの？

	やりたいことは
	・外観のスムーズな変更。
	・ItemTemplate
	・UIパーツのレイアウトをXMLで定義したい。
	・ビヘイビアをくっつけたい。
	
	後者3つはコードでもできるが・・・。
	
	特にやりたいのは ItemTemplate。
	というより、デフォルトで用意されているコントロールの内部アイテムを変えたい。
	…が、MVVMじゃないのにソースデータどうする？
	→ 普通の ListBox なら string を Add していくが、これを Control を Add するようにする。
	   (Control「も」か？)
	
	仮想化は？
	こればっかりはコールバック必須になりそう。
	WPF的にやろうとしたら UIObject.SetValue("prop", "値") みたいなことして、
	UIObject のコレクションを作らないとダメ。
	確か ToolKit EX の DataGrid も、VirtualizationCollectionView は コールバックだったし…。
	各言語用にコールバックの仕組みを整えて、そのうえで考えた方がいいと思う。
	
	WPF の Modarn-UI みたいに、テーマ切り替え時にアニメーションもしたい。
	→これはコア側の機能。変更のあったリソース名にストーリーボードを挿入する。
	
	

-------------------------------------------------------------------------------
■ 各プログラム言語でのコールバック実装方法

	

	[2014/11/27] めも
	Ruby や C# は GC の都合で、delegate とかをずっと C 側で持っておくことはできない。
	（頑張ればできるが…）
	第一、Cにコールバック関数渡せない言語では同じ仕組みを使うことができない。
	
	前提…
	PeekEvent のメッセージループはそれぞれの言語で実装し、
	取ったイベントに基づいてメソッド呼び出したりラベルジャンプしたりする。
	コールバックの呼び出しはそれぞれの言語の役目とする。
	
	※※※※
	現在の言語バインダは、例えば ViewPane.Layers プロパティのように、
	get したときに C オブジェクトに対する Managed インスタンスを "フィールドに" もっていなければ
	その場でラップするための Managed インスタンスを作る。
	つまり、別々の Managed インスタンス 2 つが同じ C オブジェクトを指すことがある。
	
	1対1で対応させるには、丁度 LNote::C_API でやってる参照管理とほぼ同じことしないとならない。
	グローバルな配列なりmapなりで、IDと Managed オブジェクトを全て管理する。
	
	ポイントは削除のタイミング。
	バインダ側にコールバックできない前提だと、C オブジェクトが C オブジェクトを
	delete するのは厳禁。Managed オブジェクトが生きている時は必ず C オブジェクトは生きていないとダメ。
	例えば「子要素を自動delete」みたいなのは絶対ダメ。(参照カウントデクリメントならOK)
	
	基本は以下の2択。
	・Dispose で Cオブジェクトの Release + グローバルmapから this を削除
	・グローバル map の Value を WeakReference で持つ。
	
	↑前者は×。Release しても、SceneNode とかはまだ親ノードからの参照が残っているかもしれない。
	
	
	Managed オブジェクトの 参照を外していいのは、C_APIManager の管理リストから Remove された後。(C側が確実に delete された後)
	それまでは Managed 側の管理リストにずっと持っておく必要がある。(参照を持っておく必要がある)
	つまり、WeakRef でもダメ。ファイナライザで Release もできない。
	
	
	
	XML から C# で作ったクラスのインスタンスを作るには？
	XMLのパーサがインスタンスを作りたいときは… コールバックしかない。
	そのコールバック (C#)では、Managed インスタンスを作って、グローバルな管理配列に入れておく。
	
	…場合によっては、C++Core 内で何か new したらその都度 callback 呼ぶような仕組みがあった方がいいのかもしれない。
	

-------------------------------------------------------------------------------
■ WPF の Visual、UIElement、FramworkElement、Control、Shape の違い

・継承関係はこんな感じ。
	Control < FramworkElement < UIElement < Visual
	Shape   < FramworkElement < UIElement < Visual
	
・Visual は以下のような要素を持つ
	・透明度
	・クリッピング領域
	・Visual 子要素
	・ヒットテスト
	・Transform
	・Viewport3DVisualやDrawingVisual(内部クラス？)に派生している。

・UIElement
	・レイアウトに関する基本情報 (measuer メソッド)
	・MouseDown や DragDrop 等ほぼ共通のイベントはここ。
	・MSDN上では派生は FramworkElement のみ

・FramworkElement
	・AcutualHeight、MaxHeight SizeChanged イベント等、サイズに関わる情報
	・DataContext
	・Style
	・ToolTip

・Control は以下のような要素を持つ
	・外観の色 (背景色等)
	・フォント
	・TabIndex
	・この Control に適用している ControlTemplate
	

-------------------------------------------------------------------------------
■



・オブジェ宇土リスト
・構造体リスト (頂点バッファとか)
・階層構造












=======



-------------------------------------------------------------------------------
■ WPF の Visual、UIElement、FramworkElement、Control、Shape の違い

・継承関係はこんな感じ。
	Control < FramworkElement < UIElement < Visual
	Shape   < FramworkElement < UIElement < Visual
	
・Visual は以下のような要素を持つ
	・透明度
	・クリッピング領域
	・Visual 子要素
	・ヒットテスト
	・Transform
	・Viewport3DVisualやDrawingVisual(内部クラス？)に派生している。

・UIElement
	・レイアウトに関する基本情報 (measuer メソッド)
	・MouseDown や DragDrop 等ほぼ共通のイベントはここ。
	・MSDN上では派生は FramworkElement のみ

・FramworkElement
	・AcutualHeight、MaxHeight SizeChanged イベント等、サイズに関わる情報
	・DataContext
	・Style
	・ToolTip

・Control は以下のような要素を持つ
	・外観の色 (背景色等)
	・フォント
	・TabIndex
	・この Control に適用している ControlTemplate
	

-------------------------------------------------------------------------------
■ 言語バインダ (GUIにかかわらず全体について)

○ラップオブジェクトはバインダ側で一意のIDを割り当て、全てのラップオブジェクトをグローバル配列で管理する。
	この ID は Cオブジェクトのユーザーデータとして割り当て、Cオブジェクトから対応するラップオブジェクトを特定するために使用する。
	以下のような処理で必要になる。
	・イベントハンドラ (コールバック)
	・get/setで対となるプロパティ以外のルートからの get (シェーダ変数への set → get 等)

	旧実装
		ラップオブジェクト → Cオブジェクト の1方向のみの参照で良ければ、一意のIDとか必要ない。
		しかし、GUIモジュール用のイベントハンドラの実装で必要になった。


-------------------------------------------------------------------------------
■ UI要素のベースオブジェクト





-------------------------------------------------------------------------------
■ Behavior


-------------------------------------------------------------------------------
■ NumUpDown をユーザーコントロールとして作ってみる。

○ C#
	class NumUpDown : UserControl
	{
		TextBox _textBox;
		RepeatButton _incrButton;
		RepeatButton _decrButton;
	
		public NumUpDown()
		{
			_textBox = new TextBox();
		}
	};




-------------------------------------------------------------------------------
■ 普通の RepertButton を各言語でユーザーコントロールとして作ってみる。
・この例の RepertButton の動作
	・背景色と文字列を設定できる。(文字列は子要素)
	・装飾用の Border を用意する。
	・private フィールド的な _updateFrames を持つ (ほんとは時間単位の方がいいけど、ここでは例として)
	・Clicked イベントを自作する。
	・マウス左クリックで押下状態になり、その後マウスボタンを離すと Click イベントが発生する。
	・ユーザーコントロールとして GUI システムに登録し、再利用できるようにする。

○ C言語
	// private な状態を保持するユーザーデータ。
	// C++ class 等の private メンバ変数にあたる。
	struct MyRepertButtonState
	{
		bool IsPressed;		// マウスダウン中
	};
	
	void main()
	{
		// ユーザーコントロールとして再利用できるようにテンプレートとして GUI システムに登録する。
		
		lnHandle myRepertButtonTemplate;
		LNUIControlTemplate_Create(&myRepertButtonTemplate);
		
		// 枠太さ4px赤枠
		lnHandle borderFactory;
		LNUIElementFactory_Create(&borderFactory, "Border");
		LNUIObject_SetName(&borderFactory, "_border");			// "_border" という名前を付ける
		LNUIElementFactory_SetValue(&borderFactory, "BorderThickness", "4");
		LNUIElementFactory_SetValue(&borderFactory, "BorderBrush", "Red");
		LNUIControlTemplate_AddChild(myRepertButtonTemplate, borderFactory);
		
		// ContentPresenter (今回の RepertButton は ContentControl として作るため、1つ必要)
		lnHandle contentPresenterFactory;
		LNUIElementFactory_Create(&contentPresenterFactory, "ContentPresenter");
		LNUIElementFactory_SetValue(&contentPresenterFactory, "HorizontalAlignment", "Center");	// この要素を中央揃え
		LNUIElementFactory_AddChild(borderFactory, contentPresenterFactory);			// Border の子として追加
		
		
		LNUIManaer_RegisterUIElement(
			"MyRepertButton",				// この名前で登録する
			"ContentControl",			// ベース要素は ContentControl
			myRepertButtonTemplate);		// create 時にこのテンプレートを適用する
		
		
		lnHandle button;
		LNUIManaer_CreateUIElement(&button, "MyRepertButton");
		
		do
		{
			while (LNUIManager_PeekEvent(&e))
			{
				lnHandle sender;
				LNUIEvent_GetSender(e, &sender);
				
				const char* typeName;
				LNUIElement_GetTypeName(e, &typeName);
				
				if (strcmp(typeName, "MyRepertButton"))
				{
					lnEventType et;
					LNUIEvent_GetEventType(e, &et);
					
					if (et == LN_EVENT_MOUSE_DOWN)
					{
						LNUIEvent_GetEventType(e, &et);
						LNUIElement_GoToVisualState("Pressed");		// Pressed 状態へ移動 (この名前の状態が無ければ何もしない)
					}
				}
			}
		
		} while(update):
	}


○ C++

○ C#



-------------------------------------------------------------------------------
■ OnRender

*/
#include "../Internal.h"
#include <Lumino/GUI/ControlTemplate.h>
#include <Lumino/GUI/UIElement.h>
#include <Lumino/GUI/Controls/Thumb.h>
#include <Lumino/GUI/Controls/Track.h>
#include <Lumino/GUI/Controls/Grid.h>
#include <Lumino/GUI/Controls/Image.h>
#include <Lumino/GUI/Controls/ScrollBar.h>
#include <Lumino/GUI/Controls/ListBox.h>
#include <Lumino/GUI/GUIManager.h>

namespace Lumino
{
namespace GUI
{
	
//=============================================================================
// GUIManager
//=============================================================================

static const byte_t g_DefaultSkin_png_Data[] =
{
#include "Resource/DefaultSkin.png.h"
};
static const size_t g_DefaultSkin_png_Len = LN_ARRAY_SIZE_OF(g_DefaultSkin_png_Data);

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GUIManager::GUIManager()
	: m_mouseHoverElement(NULL)
	, m_defaultTheme(NULL)
	, m_rootCombinedResource(NULL)
	, m_defaultRootPane(NULL)
	, m_capturedElement(NULL)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GUIManager::~GUIManager()
{
	LN_SAFE_RELEASE(m_rootCombinedResource);
	LN_SAFE_RELEASE(m_defaultTheme);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUIManager::Initialize(const ConfigData& configData)
{
	if (LN_VERIFY_ASSERT(configData.GraphicsManager != NULL)) { return; }

	m_graphicsManager = configData.GraphicsManager;
	m_mainWindow = configData.MainWindow;


	RegisterFactory(ContentPresenter::TypeID, ContentPresenter::CreateInstance);
	RegisterFactory(ItemsPresenter::TypeID, ItemsPresenter::CreateInstance);
	RegisterFactory(ButtonChrome::TypeID, ButtonChrome::CreateInstance);
	RegisterFactory(Button::TypeID, Button::CreateInstance);
	RegisterFactory(ListBoxChrome::TypeID, ListBoxChrome::CreateInstance);
	RegisterFactory(ThumbChrome::TypeID, ThumbChrome::CreateInstance);
	RegisterFactory(Thumb::TypeID, Thumb::CreateInstance);
	RegisterFactory(Track::TypeID, Track::CreateInstance);
	RegisterFactory(Grid::TypeID, Grid::CreateInstance);
	RegisterFactory(ColumnDefinition::TypeID, ColumnDefinition::CreateInstance);
	RegisterFactory(RowDefinition::TypeID, RowDefinition::CreateInstance);
	RegisterFactory(Image::TypeID, Image::CreateInstance);
	RegisterFactory(ScrollBar::TypeID, ScrollBar::CreateInstance);

	

	m_defaultTheme = LN_NEW ResourceDictionary();
	BuildDefaultTheme();

	m_rootCombinedResource = LN_NEW CombinedLocalResource();
	m_rootCombinedResource->Combine(NULL, m_defaultTheme);




	m_defaultRootPane = LN_NEW RootPane(this);
	m_defaultRootPane->ApplyTemplate();	// テーマを直ちに更新
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUIManager::Finalize()
{
	LN_SAFE_RELEASE(m_defaultRootPane);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
//RootPane* GUIManager::CreateRootPane()
//{
//	m_defaultRootPane = LN_NEW RootPane(this);
//	m_defaultRootPane->ApplyTemplate(m_rootCombinedResource);	// テーマを直ちに更新
//	return m_rootPane;
//}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUIManager::RegisterFactory(const String& typeFullName, ObjectFactory factory)
{
	m_objectFactoryMap[typeFullName] = factory;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CoreObject* GUIManager::CreateObject(const String& typeFullName)
{
	ObjectFactory f = m_objectFactoryMap[typeFullName];
	LN_THROW(f != NULL, KeyNotFoundException, typeFullName.GetCStr());
	return f(this);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool GUIManager::InjectMouseMove(float clientX, float clientY)
{
	// キャプチャ中のコントロールがあればそちらに送る
	if (m_capturedElement != NULL) 
	{
		RefPtr<MouseEventArgs> args(m_eventArgsPool.CreateMouseEventArgs(MouseButton_None, 0, clientX, clientY));
		return m_capturedElement->OnEvent(EventType_MouseMove, args);
	}
	//if (m_defaultRootPane == NULL) { return false; }
	UpdateMouseHover(PointF(clientX, clientY));
	if (m_mouseHoverElement == NULL) { return false; }
	RefPtr<MouseEventArgs> args(m_eventArgsPool.CreateMouseEventArgs(MouseButton_None, 0, clientX, clientY));
	//if (m_mouseHoverElement != NULL)
	return m_mouseHoverElement->OnEvent(EventType_MouseMove, args);
	//bool r = m_defaultRootPane->OnEvent(EventType_MouseMove, args);
	//return UpdateMouseHover(PointF(clientX, clientY));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool GUIManager::InjectMouseButtonDown(MouseButton button, float clientX, float clientY)
{
	// キャプチャ中のコントロールがあればそちらに送る
	if (m_capturedElement != NULL)
	{
		RefPtr<MouseEventArgs> args(m_eventArgsPool.CreateMouseEventArgs(button, 0, clientX, clientY));
		return m_capturedElement->OnEvent(EventType_MouseButtonDown, args);
	}
	if (m_mouseHoverElement == NULL) { return false; }
	RefPtr<MouseEventArgs> args(m_eventArgsPool.CreateMouseEventArgs(button, 0, clientX, clientY));
	//if (m_mouseHoverElement != NULL) {
	return m_mouseHoverElement->OnEvent(EventType_MouseButtonDown, args);
	//}
	//return m_defaultRootPane->OnEvent(EventType_MouseButtonDown, args);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool GUIManager::InjectMouseButtonUp(MouseButton button, float clientX, float clientY)
{
	// キャプチャ中のコントロールがあればそちらに送る
	if (m_capturedElement != NULL)
	{
		RefPtr<MouseEventArgs> args(m_eventArgsPool.CreateMouseEventArgs(button, 0, clientX, clientY));
		return m_capturedElement->OnEvent(EventType_MouseButtonUp, args);
	}
	//if (m_defaultRootPane == NULL) { return false; }
	if (m_mouseHoverElement == NULL) { return false; }
	RefPtr<MouseEventArgs> args(m_eventArgsPool.CreateMouseEventArgs(button, 0, clientX, clientY));
	//return m_defaultRootPane->OnEvent(EventType_MouseButtonUp, args);
	return m_mouseHoverElement->OnEvent(EventType_MouseButtonUp, args);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool GUIManager::InjectMouseWheel(int delta, float clientX, float clientY)
{
	// キャプチャ中のコントロールがあればそちらに送る
	if (m_capturedElement != NULL)
	{
		RefPtr<MouseEventArgs> args(m_eventArgsPool.CreateMouseEventArgs(MouseButton_None, 0, clientX, clientY));
		return m_capturedElement->OnEvent(EventType_MouseMove, args);
	}
	//if (m_defaultRootPane == NULL) { return false; }
	if (m_mouseHoverElement == NULL) { return false; }
	RefPtr<MouseEventArgs> args(m_eventArgsPool.CreateMouseEventArgs(MouseButton_None, delta, clientX, clientY));
	//return m_defaultRootPane->OnEvent(EventType_MouseWheel, args);
	return m_mouseHoverElement->OnEvent(EventType_MouseWheel, args);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool GUIManager::InjectKeyDown(Key keyCode, bool isAlt, bool isShift, bool isControl)
{
	if (m_defaultRootPane == NULL) { return false; }
	RefPtr<KeyEventArgs> args(m_eventArgsPool.CreateKeyEventArgs(keyCode, isAlt, isShift, isControl));
	return m_defaultRootPane->OnEvent(EventType_KeyDown, args);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool GUIManager::InjectKeyUp(Key keyCode, bool isAlt, bool isShift, bool isControl)
{
	if (m_defaultRootPane == NULL) { return false; }
	RefPtr<KeyEventArgs> args(m_eventArgsPool.CreateKeyEventArgs(keyCode, isAlt, isShift, isControl));
	return m_defaultRootPane->OnEvent(EventType_KeyUp, args);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool GUIManager::InjectElapsedTime(float elapsedTime)
{
	LN_THROW(0, NotImplementedException);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUIManager::CaptureMouse(UIElement* element)
{
	m_capturedElement = element;
	m_mainWindow->CaptureMouse();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUIManager::ReleaseMouseCapture(UIElement* element)
{
	if (m_capturedElement == element)
	{
		m_capturedElement = NULL;
		m_mainWindow->ReleaseMouseCapture();
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool GUIManager::UpdateMouseHover(const PointF& mousePos)
{
	UIElement* old = m_mouseHoverElement;

	// TODO:IME側のイベントを処理する
	//if ( m_pIme != NULL )
	//{
	//	if ( m_pIme->OnMouseHoverCheck( m_MousePosition, &mMouseHoverControl ) )
	//	{
	//		goto EXIT;
	//	}
	//}

	// 通常のウィンドウのイベントを処理する
	if (m_defaultRootPane != NULL)
	{
		m_mouseHoverElement = m_defaultRootPane->CheckMouseHoverElement(mousePos);
		if (m_mouseHoverElement != NULL) {
			goto EXIT;
		}
	}

	m_mouseHoverElement = NULL;

EXIT:
	// 新旧それぞれの Element に MouseLeave、MouseEnter イベントを送る
	if (m_mouseHoverElement != old)
	{
		RefPtr<MouseEventArgs> args(m_eventArgsPool.CreateMouseEventArgs(MouseButton_None, 0, mousePos.X, mousePos.Y));
		if (old != NULL) {
			old->OnEvent(EventType_MouseLeave, args);
		}
		if (m_mouseHoverElement != NULL) {
			return m_mouseHoverElement->OnEvent(EventType_MouseEnter, args);
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GUIManager::BuildDefaultTheme()
{
	// RootPane
	{
		RefPtr<ControlTemplate> t(LN_NEW ControlTemplate());
		t->SetTargetType(_T("RootPane"));

		RefPtr<UIElementFactory> presenter1(LN_NEW UIElementFactory(this));
		presenter1->SetTypeName(_T("ContentPresenter"));
		t->SetVisualTreeRoot(presenter1);

		m_defaultTheme->AddControlTemplate(t);
	}

	// Brush (ボタン枠)
	{
		RefPtr<Graphics::TextureBrush> obj(LN_NEW Graphics::TextureBrush());	//TODO:
		obj->Create(_T("../../src/GUI/Resource/DefaultSkin.png"), m_graphicsManager);
		obj->SetSourceRect(Rect(0, 0, 32, 32));
		m_defaultTheme->AddItem(_T("ButtonNormalFrameBrush"), obj);
	}
	// Brush (ボタン背景)
	{
		RefPtr<Graphics::TextureBrush> obj(LN_NEW Graphics::TextureBrush());	//TODO:
		obj->Create(_T("../../src/GUI/Resource/DefaultSkin.png"), m_graphicsManager);
		obj->SetSourceRect(Rect(8, 8, 16, 16));
		m_defaultTheme->AddItem(_T("ButtonNormalBackgroundBrush"), obj);
	}
	// Brush (ListBox 枠)
	{
		RefPtr<Graphics::TextureBrush> obj(LN_NEW Graphics::TextureBrush());	//TODO:
		obj->Create(_T("../../src/GUI/Resource/DefaultSkin.png"), m_graphicsManager);
		obj->SetSourceRect(Rect(0, 32, 32, 32));
		m_defaultTheme->AddItem(_T("ListBoxNormalFrameBrush"), obj);
	}
	// Brush (ListBox 背景)
	{
		RefPtr<Graphics::TextureBrush> obj(LN_NEW Graphics::TextureBrush());	//TODO:
		obj->Create(_T("../../src/GUI/Resource/DefaultSkin.png"), m_graphicsManager);
		obj->SetSourceRect(Rect(8, 32 + 8, 16, 16));
		m_defaultTheme->AddItem(_T("ListBoxNormalBackgroundBrush"), obj);
	}

	// Button
	{
		RefPtr<ControlTemplate> t(LN_NEW ControlTemplate());
		t->SetTargetType(_T("Button"));

		RefPtr<UIElementFactory> ef1(LN_NEW UIElementFactory(this));
		ef1->SetTypeName(_T("ButtonChrome"));
		ef1->AddTemplateBinding(ButtonChrome::IsMouseOverProperty, Button::IsMouseOverProperty);
		t->SetVisualTreeRoot(ef1);

		RefPtr<UIElementFactory> ef2(LN_NEW UIElementFactory(this));
		ef2->SetTypeName(_T("ContentPresenter"));
		ef1->AddChild(ef2);

		m_defaultTheme->AddControlTemplate(t);
	}

	// ListBox
	{
		RefPtr<ControlTemplate> t(LN_NEW ControlTemplate());
		t->SetTargetType(_T("ListBox"));

		RefPtr<UIElementFactory> ef1(LN_NEW UIElementFactory(this));
		ef1->SetTypeName(_T("ListBoxChrome"));
		//ef1->AddTemplateBinding(ButtonChrome::IsMouseOverProperty, Button::IsMouseOverProperty);
		t->SetVisualTreeRoot(ef1);

		RefPtr<UIElementFactory> ef2(LN_NEW UIElementFactory(this));
		ef2->SetTypeName(_T("ItemsPresenter"));
		ef1->AddChild(ef2);

		m_defaultTheme->AddControlTemplate(t);
	}

	// Thumb (枠 Brush)
	{
		RefPtr<Graphics::TextureBrush> obj(LN_NEW Graphics::TextureBrush());
		obj->Create(_T("../../src/GUI/Resource/DefaultSkin.png"), m_graphicsManager);
		obj->SetSourceRect(Rect(0, 64, 32, 32));
		m_defaultTheme->AddItem(_T("ThumbChromeBackgroundFrameBrush"), obj);
	}
	// Thumb (枠背景 Brush)
	{
		RefPtr<Graphics::TextureBrush> obj(LN_NEW Graphics::TextureBrush());
		obj->Create(_T("../../src/GUI/Resource/DefaultSkin.png"), m_graphicsManager);
		obj->SetSourceRect(Rect(8, 64 + 8, 16, 16));
		m_defaultTheme->AddItem(_T("ThumbChromeBackgroundInnerBrush"), obj);
	}
	// Thumb
	{
		RefPtr<ControlTemplate> t(LN_NEW ControlTemplate());
		t->SetTargetType(_T("Thumb"));

		RefPtr<UIElementFactory> ef1(LN_NEW UIElementFactory(this));
		ef1->SetTypeName(_T("ThumbChrome"));
		//ef1->AddTemplateBinding(ButtonChrome::IsMouseOverProperty, Button::IsMouseOverProperty);
		t->SetVisualTreeRoot(ef1);

		m_defaultTheme->AddControlTemplate(t);
	}

	// Track
	{
		RefPtr<ControlTemplate> t(LN_NEW ControlTemplate());
		t->SetTargetType(_T("Track"));

		RefPtr<UIElementFactory> button1(LN_NEW UIElementFactory(this));
		button1->SetTypeName(_T("Button"));
		t->SetPropertyValue(Track::DecreaseButtonProperty, button1);

		RefPtr<UIElementFactory> thumb1(LN_NEW UIElementFactory(this));
		thumb1->SetTypeName(_T("Thumb"));
		t->SetPropertyValue(Track::ThumbProperty, thumb1);

		RefPtr<UIElementFactory> button2(LN_NEW UIElementFactory(this));
		button2->SetTypeName(_T("Button"));
		t->SetPropertyValue(Track::IncreaseButtonProperty, button2);

		m_defaultTheme->AddControlTemplate(t);
	}

	// ScrollBar
	{

		RefPtr<ControlTemplate> scrollBar(LN_NEW ControlTemplate());
		scrollBar->SetTargetType(_T("ScrollBar"));

		//RefPtr<UIElementFactory> button1(LN_NEW UIElementFactory(this));
		//button1->SetTypeName(_T("Button"));
		//t->SetPropertyValue(Track::DecreaseButtonProperty, button1);

		RefPtr<UIElementFactory> thumb1(LN_NEW UIElementFactory(this));
		thumb1->SetTypeName(_T("Track"));
		scrollBar->SetVisualTreeRoot(thumb1);

		//RefPtr<UIElementFactory> button2(LN_NEW UIElementFactory(this));
		//button2->SetTypeName(_T("Button"));
		//t->SetPropertyValue(Track::IncreaseButtonProperty, button2);

		m_defaultTheme->AddControlTemplate(scrollBar);
	}

}

} // namespace GUI
} // namespace Lumino
