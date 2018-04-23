文字列の書式指定
==========
C++ では書式を指定してオブジェクトと文字列を結合して出力するために printf や iostream を使用しますが、
Lumino はそれらの安全で柔軟な代替手段として、python や .NET のような複合書式文字列を使用する方法を提供します。


クイックスタート
----------
文字列の書式指定には `String::format` を使用するのが簡単です。

この関数は書式として、.NET で使用されている複合書式文字列に似た書式を受け取ります。

```cpp
String prefix = "file";
int index = 5;
String fileName = String::format("{}-{}.txt", prefix, index);   // => file-5.txt
```


書式文字列の構文
----------
```
{[index][,alignment][:formatString][precision]}
```
* index
    * 引数リストに指定された値の番号。
* alignment
    * フィールドの幅。書式設定された文字列よりも長い場合空白で埋められ、値が正の場合は右揃え、負の場合は左揃えになります。
* formatString
    * 書式指定文字列。以下のセクションを参照してください。
* precision
    * 精度指定子。formatString によって意味が変わります。


### 引数リストの番号

{} 内に数字を指定することで、引数リストのどの値を埋め込むかを選択できます。

```cpp
String::format("{0}-{1}-{0}", "foo", "bar"));   // => "foo-bar-foo"
```

省略した場合は引数リストを順に埋め込みます。

```cpp
String::format("{}-{}", "foo", "bar"));   // => "foo-bar"
```

### 10 進数 ("D") 書式指定子

数値を 10 進数文字列に変換します。入力は整数型のみサポートします。

精度指定子は変換後の文字列の最小桁数です。
出力がこの桁数未満の場合は0埋めを行います。

```cpp
String::format("{0:D}", 12345));   // => "12345"
String::format("{0:d}", -12345));  // => "-12345"
String::format("{0:D8}", 12345));  // => "00012345"
```

### 16 進数 ("X") 書式指定子

数値を 16 進数文字列に変換します。入力は整数型のみサポートします。
書式指定子が大文字か小文字かによって出力される文字列の大文字か小文字が決まります。

精度指定子は変換後の文字列の最小桁数です。
出力がこの桁数未満の場合は0埋めを行います。

```cpp
String::format("{0:x}", 0x2045e);  // => "2045e"
String::format("{0:X}", 0x2045e);  // => "2045E"
String::format("{0:X8}", 0x2045e); // => "0002045E"
String::format("0x{0:X}", 255);    // => "0xFF"
```

### 固定小数点 ("F") 書式指定子

実数を固定小数点の文字列に変換します。

精度指定子は小数部の桁数です。

```cpp
String::format("{0:F}", 25.1879));                 // => "25.1879"
String::format("{0:f}", 25.1879));                 // => "25.1879"
String::formatu("{0:F2}", 25.1879));                // => "25.19"
String::format(Locale("fr"), "{0:F2}", 25.1879)); // => "25,187900"
```


### 指数 ("E") 書式指定子

実数を指数表現の文字列に変換します。
書式指定子が大文字か小文字かによって出力される文字列の大文字か小文字が決まります。

精度指定子は小数部の桁数です。

```cpp
String::format("{0:e}", 12345.6789);                // => "1.234568e+004"
String::format("{0:E10}", 12345.6789);              // => "1.2345678900E+004"
String::format("{0:e4}", 12345.6789);               // => "1.2346e+004"
String::format(Locale("fr"), "{0:E}", 12345.6789); // => "1,234568E+004"
```


### {} のエスケープ

左中かっこ ({) および右中かっこ (}) は、書式指定項目の開始および終了として解釈されます。
したがって、左中かっこおよび右中かっこを文字として表示するためには、エスケープ シーケンスを使用する必要があります。
左中かっこを 1 つ ("{") 表示するには、左中かっこ 2 つ ("{{") を固定テキストに指定します。

```cpp
String::format("{{0}}");         // => "{0}"
String::format("{{{0}}}", 1);    // => "{1}"
```


型安全性
----------

引数リストに指定された値の型が受け入れられない場合、 コンパイルエラーによってコンパイル時に不正を検出します。

また、型のサイズは引数リストに指定された値の型から求めるため printf 書式の移植性の問題もありません。