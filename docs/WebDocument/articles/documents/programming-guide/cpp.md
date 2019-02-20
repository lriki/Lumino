Lumino の C++ プログラミングガイド
==========

メモリ管理
----------

C++ では通常、new したメモリはプログラマが責任をもって delete しなければメモリリークが発生してしまいます。メモリリークは代表的な問題ですが、C++ はそれ以外にもメモリやポインタに起因する不具合を作りこんでしまいやすいと言われています。

### スマートポインタについて

そういった問題の対策のひとつとして、確保したメモリを自動的に解放してくれるクラスが考え出されました。このようなクラスを「スマートポインタ」と呼びます。

確保したメモリのポインタをスマートポインタに渡すと、スマートポインタはそのメモリに対する所有権を取得します。

確保したメモリは、いずれかのスマートポインタが所有権を保持している間は解放されません。
所有権を持つスマートポインタが全て破棄されれば、その時点で解放されます。

C++11 以降、`shared_ptr<T>` クラスなどのスマートポインタクラスが標準ライブラリとして追加されています。


### Lumino のスマートポインタについて

Lumino のスマートポインタは `Ref<T>` クラスです。

Lumino のオブジェクト (ln::RefObject のサブクラス) は、オブジェクト自身が参照カウントという値を持ちます。これによって何個のスマートポインタから所有されているかを表します。

基本的な使い方は次のようになります。

```
class MyObject : public ln::RefObject   // RefObject を継承してクラスを定義する
{};

void Main()
{
    // MyObject のインスタンスを作成し、その所有権を持つ ptr を作成
    Ref<MyObject> ptr = ln::makeRef<MyObject>();

} // ここで ptr のデストラクタが呼ばれて所有権を持つ Ref がいなくなったので、MyObject のインスタンスが解放される
```


### Lumino におけるスマートポインタと通常ポインタの使い分けについて

何らかのオブジェクトを作成して返す関数は、必ずスマートポインタを返します。
例えば、Assets クラスの loadTexture メソッドの宣言は次のようになっています。

```
static Ref<Texture2D> loadTexture(const StringRef& filePath);
```

一方、既に Lumino のエンジン内部やほかのオブジェクトが保持しているオブジェクトを返すだけの関数は、通常のポインタを返します。
例えば、Engine::initialize() 時に内部で作成されるカメラを返す、Engine クラスの mainCamera() メソッドの宣言は次のようになっています。

```
static Camera* mainCamera();
```

どちらにしても、返ってきたポインタの delete は不要です。
















