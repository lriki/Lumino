Lumino の基本
==========

Note:
- Lumino の基本
- 



最小のプログラム
----------

「最初のプログラム」で見たように、Lumino でアプリを開発するには Application クラスの実装から始めます。

空のウィンドウを表示するだけの最小限のプログラムは、次のようになります。

# [C++](#tab/lang-cpp)

```cpp
#include <Lumino.hpp>

class App : public Application
{
};

LUMINO_APP(App);
```

# [Ruby](#tab/lang-ruby)

```ruby
require 'lumino'

class App < Application
end

App.new.run
```

---

実行して、ウィンドウを表示してみましょう。

![](img/basic-1.png)

この時点でできることは、クローズボタンや Esc キーでウィンドウを閉じるだけです。

文字を表示したり、ユーザー入力を受けてインタラクションを実現するためにはこの App クラスにいくつかのメソッドを実装する必要があります。


初期化と更新
----------

小さな Lumino アプリケーションを作成するための基本的なタスクは次の2つです。

- プログラムの開始時に変数を初期化する
- プログラムが動き出したら、繰り返し変数を変更する

これらを行うために、次のように 2 つのメソッド定義を追加します。

# [C++](#tab/lang-cpp)

```cpp
#include <Lumino.hpp>

class App : public Application
{
	virtual void onInit() override
	{
	}

	virtual void onUpdate() override
	{
	}
};

LUMINO_APP(App);
```

# [Ruby](#tab/lang-ruby)

```ruby
require 'lumino'

class App < Application
    def on_init
    end

    def on_update
    end
end

App.new.run
```

---

空のメソッドを追加しただけなので、実行するとウィンドウは表示できますが、動きは変わりません。

この後のチュートリアルやサンプルで紹介するプログラムでは、プログラムの開始時に1回呼び出される onInit() メソッドで変数を初期化し、onUpdate() メソッドでこれらの変数を変更して、アプリケーションを実装していくことになります。

> [!Note]
> onUpdate() は 1秒間に 60 回、繰り返し実行されます。


Hello, Lumino!
----------

ウィンドウに文字列を表示してみましょう。

テキストや数値を画面に表示するには、Lumino の デバッグようの機能である、Debug クラスの print() メソッドを使うと簡単にできます。

# [C++](#tab/lang-cpp)

```cpp
#include <Lumino.hpp>

class App : public Application
{
	virtual void onInit() override
	{
		Debug::print(u"Hello, Lumino!");
	}

	virtual void onUpdate() override
	{
	}
};

LUMINO_APP(App);
```

# [Ruby](#tab/lang-ruby)

```ruby
ruby
```

---

文字列がウィンドウ上に表示されます。（その後、しばらくすると消えます）

![](img/basic-2.png)


文字列を表示し続ける
----------

次は onUpdate で文字列を表示してみます。

# [C++](#tab/lang-cpp)
```cpp
#include <Lumino.hpp>

class App : public Application
{
	virtual void onInit() override
	{
	}

	virtual void onUpdate() override
	{
		Debug::print(String::format(u"Time: {0}", Engine::time()));
	}
};

LUMINO_APP(App);
```
# [Ruby](#tab/lang-ruby)
```ruby
ruby
```
---

`Engine::time()` は アプリケーションの起動からの経過時間を返します。これを利用して、onUpdate() がどのくらいの頻度で実行されているのかを確認してみます。

![](img/basic-3.png)

実行してみると、画面からあふれるほどのテキストが表示されてしまいました。

繰り返し実行されているのはわかりましたが、今は過去の情報は不要です。

`Debug::print()` は第一引数に数値を指定することで、テキストの表示時間をコントロールできます。次のように修正して、実行してみましょう。

# [C++](#tab/lang-cpp)
```cpp
#include <Lumino.hpp>

class App : public Application
{
	virtual void onInit() override
	{
	}

	virtual void onUpdate() override
	{
		Debug::print(0, String::format(u"Time: {0}", Engine::time()));
	}
};

LUMINO_APP(App);
```
# [Ruby](#tab/lang-ruby)
```ruby
ruby
```
---


![](img/basic-4.gif)

シンプルなタイマーができました！