﻿
#include <errno.h>
#include <stdarg.h>
#include <wchar.h>
#include <math.h>
#include <float.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <LuminoCore/Base/Assertion.hpp>
#include <LuminoCore/Base/StringHelper.hpp>
#include <LuminoCore/Base/StlHelper.hpp>

#ifdef LN_MSVC
#else
#include <xlocale.h>
#endif

namespace ln {

template<typename TChar>
static int my_strncmp(const TChar* str1, const TChar* str2, size_t count) LN_NOEXCEPT
{
	for (; 0 < count; --count, ++str1, ++str2)
	{
		if (*str1 != *str2)
		{
			return (*str1 < *str2) ? -1 : 1;
		}
	}
	return 0;
}

template<typename TChar>
static int my_strnicmp(const TChar* str1, const TChar* str2, size_t count) LN_NOEXCEPT
{
	for (; 0 < count; --count, ++str1, ++str2)
	{
		TChar c1 = StringHelper::toUpper(*str1);
		TChar c2 = StringHelper::toUpper(*str2);
		if (c1 != c2)
		{
			return (c1 < c2) ? -1 : 1;
		}
	}
	return 0;
}

//==============================================================================
// StringHelper

#ifdef _WIN32
int StringHelper::vsprintf(char* out, int charCount, const char* format, va_list args) { return _vsnprintf_s(out, charCount, _TRUNCATE, format, args); }
int StringHelper::vsprintf(wchar_t* out, int charCount, const wchar_t* format, va_list args) { return vswprintf(out, charCount, format, args); }
#else
int StringHelper::vsprintf(char* out, int charCount, const char* format, va_list args) { return vsnprintf(out, charCount, format, args); }
int StringHelper::vsprintf(wchar_t* out, int charCount, const wchar_t* format, va_list args)
{
	LN_NOTIMPLEMENTED();	// vswprintf は動作保障無し
	return vswprintf(out, charCount, format, args);
}
#endif

int StringHelper::vsprintf2(char* out, int charCount, const char* format, ...)
{
	va_list list;
	va_start(list, format);
	int r = vsprintf(out, charCount, format, list);
	va_end(list);
	return r;
}

int StringHelper::vsprintf2(wchar_t* out, int charCount, const wchar_t* format, ...)
{
	va_list list;
	va_start(list, format);
	int r = vsprintf(out, charCount, format, list);
	va_end(list);
	return r;
}

template<typename TChar>
TChar StringHelper::toUpper(TChar ch)
{
	if ('a' <= ch && ch <= 'z')
		return 'A' + (ch - 'a');
	return ch;
}
template char StringHelper::toUpper<char>(char ch);
template wchar_t StringHelper::toUpper<wchar_t>(wchar_t ch);
//template UTF8 StringHelper::toUpper<UTF8>(UTF8 ch);
//template UTF16 StringHelper::toUpper<UTF16>(UTF16 ch);
//template UTF32 StringHelper::toUpper<UTF32>(UTF32 ch);
template char16_t StringHelper::toUpper<char16_t>(char16_t ch);

template<typename TChar>
TChar StringHelper::toLower(TChar ch)
{
	if ('A' <= ch && ch <= 'Z')
		return 'a' + (ch - 'A');
	return ch;
}
template char StringHelper::toLower<char>(char ch);
template wchar_t StringHelper::toLower<wchar_t>(wchar_t ch);
//template UTF8 StringHelper::toLower<UTF8>(UTF8 ch);
//template UTF16 StringHelper::toLower<UTF16>(UTF16 ch);
//template UTF32 StringHelper::toLower<UTF32>(UTF32 ch);
template char16_t StringHelper::toLower<char16_t>(char16_t ch);

//template<typename TChar>
//bool StringHelper::isSpace(TChar ch)
//{
//	return isspace((uint8_t)ch) != 0;
//}
//template bool StringHelper::isSpace(UTF8 ch);
//template bool StringHelper::isSpace(UTF16 ch);
//template bool StringHelper::isSpace(UTF32 ch);


template<typename TChar>
int StringHelper::indexOf(const TChar* str1, int str1Len, const TChar* str2, int str2Len, int startIndex, CaseSensitivity cs)
{
	if (LN_ENSURE(str1 && str2)) return -1;

	if (*str1 == 0) {
		return -1;
	}

	str1Len = (str1Len < 0) ? ((int)strlen(str1)) : str1Len;
	if (str1Len <= startIndex) { return -1; }

	str2Len = (str2Len < 0) ? ((int)strlen(str2)) : str2Len;
	if (str2Len <= 0) { return -1; }

	const TChar* pos = str1 + startIndex;

	// 大文字小文字を区別する
	if (cs == CaseSensitivity::CaseSensitive)
	{
		for (; *pos; ++pos)
		{
			if (*pos == *str2)
			{
				if (my_strncmp(pos, str2, str2Len) == 0)
				{
					return (int)(pos - str1);
				}
			}
		}
	}
	// 大文字小文字を区別しない
	else
	{
		for (; *pos; ++pos)
		{
			if (my_strnicmp(pos, str2, str2Len) == 0)
			{
				return (int)(pos - str1);
			}
		}
	}

	return -1;
}
template int StringHelper::indexOf<char>(const char* str1, int str1Len, const char* str2, int str2Len, int startIndex, CaseSensitivity cs);
template int StringHelper::indexOf<wchar_t>(const wchar_t* str1, int str1Len, const wchar_t* str2, int str2Len, int startIndex, CaseSensitivity cs);
template int StringHelper::indexOf<char16_t>(const char16_t* str1, int str1Len, const char16_t* str2, int str2Len, int startIndex, CaseSensitivity cs);

template<typename TChar>
int StringHelper::lastIndexOf(const TChar* str1, int str1Len, const TChar* str2, int str2Len, int startIndex, int count, CaseSensitivity cs)
{
	const TChar nullStr[] = { '\0' };

	str1 = (str1 == nullptr) ? nullStr : str1;
	str2 = (str2 == nullptr) ? nullStr : str2;
	str1Len = static_cast<int>((str1Len < 0) ? strlen(str1) : str1Len);
	str2Len = static_cast<int>((str2Len < 0) ? strlen(str2) : str2Len);
	startIndex = (startIndex < 0) ? (str1Len - 1) : startIndex;

	// 検索対象文字列の長さが 0 の場合は特別な前提処理
	if (str1Len == 0 && (startIndex == -1 || startIndex == 0)) {
		return (str2Len == 0) ? 0 : -1;
	}

	if (LN_ENSURE(startIndex >= 0)) return -1;			// startIndex は 0 以上でなければならない。
	if (LN_ENSURE(startIndex < str1Len)) return -1;		// startIndex は str1 の長さを超えてはならない。

														// 検索文字数が 0 の場合は必ず検索開始位置でヒットする (strstr と同じ動作)
	if (str2Len == 0 && count >= 0 && startIndex - count + 1 >= 0) {
		return startIndex;
	}

	const TChar* pos = str1 + startIndex;							// 検索範囲の末尾の文字を指す。
	const TChar* end = (count < 0) ? str1 : pos - (count - 1);		// 検索範囲の先頭の文字を指す。
	if (LN_ENSURE(end <= pos)) return -1;						// 末尾と先頭が逆転してないこと。

	if (pos - end < (str2Len - 1)) {
		return -1;	// 検索範囲が検索文字数よりも少ない場合は見つかるはずがない
	}

	pos -= (str2Len - 1);

	// 大文字小文字を区別する
	if (cs == CaseSensitivity::CaseSensitive)
	{
		// 後ろから前へ見ていく
		while (pos >= end)
		{
			if (my_strncmp(pos, str2, str2Len) == 0) {
				return (int)(pos - str1);
			}
			--pos;
		}
	}
	// 大文字小文字を区別しない
	else
	{
		// 後ろから前へ見ていく
		while (pos >= end)
		{
			if (my_strnicmp(pos, str2, str2Len) == 0) {
				return (int)(pos - str1);
			}
			--pos;
		}
	}
	return -1;
}
template int StringHelper::lastIndexOf<char>(const char* str1, int str1Len, const char* str2, int str2Len, int startIndex, int count, CaseSensitivity cs);
template int StringHelper::lastIndexOf<wchar_t>(const wchar_t* str1, int str1Len, const wchar_t* str2, int str2Len, int startIndex, int count, CaseSensitivity cs);
template int StringHelper::lastIndexOf<char16_t>(const char16_t* str1, int str1Len, const char16_t* str2, int str2Len, int startIndex, int count, CaseSensitivity cs);

template<typename TChar>
int StringHelper::compare(const TChar* str1, int str1Len, const TChar* str2, int str2Len, int count, CaseSensitivity cs)
{
	if (str1 == nullptr || str2 == nullptr)
	{
		if (str1 == nullptr && str2 == nullptr) {
			return 0;
		}
		else if (str1 == nullptr) {
			return -1;		// NULL < ""
		}
		else {
			return 1;		// "" > NULL
		}
	}

	// 必要があれば文字数カウント
	str1Len = (str1Len < 0) ? static_cast<int>(strlen(str1)) : str1Len;
	str2Len = (str2Len < 0) ? static_cast<int>(strlen(str2)) : str2Len;
	int minCount = std::min(str1Len, str2Len);
	int maxCount = std::max(str1Len, str2Len);

	// チェックする文字数
	if (count < 0)
	{
		count = maxCount;
	}
	else
	{
		maxCount = count;
		count = LN_MIN(minCount, count);
	}

	if (cs == CaseSensitivity::CaseSensitive)
	{
		while (count > 0)
		{
			if (*str1 != *str2) {
				return ((unsigned int)(*str1)) - ((unsigned int)(*str2));
			}
			//if (*str1 == 0) {
			//	break;
			//}
			++str1;
			++str2;
			--count;
		}
	}
	else
	{
		while (count > 0)
		{
			if (toUpper(*str1) != toUpper(*str2)) {
				return ((unsigned int)(toUpper(*str1))) - ((unsigned int)(toUpper(*str2)));
			}
			//if (*str1 == 0) {
			//	break;
			//}
			++str1;
			++str2;
			--count;
		}
	}

	if (minCount < maxCount)
	{
		return str1Len - str2Len;
	}
	return 0;
}
template int StringHelper::compare<char>(const char* str1, int str1Len, const char* str2, int str2Len, int count, CaseSensitivity cs);
template int StringHelper::compare<wchar_t>(const wchar_t* str1, int str1Len, const wchar_t* str2, int str2Len, int count, CaseSensitivity cs);
template int StringHelper::compare<char16_t>(const char16_t* str1, int str1Len, const char16_t* str2, int str2Len, int count, CaseSensitivity cs);

template<typename TChar>
int StringHelper::compare(const TChar* str1, const TChar* str2, int count, CaseSensitivity cs)
{
	return compare(str1, -1, str2, -1, count, cs);
}
template int StringHelper::compare<char>(const char* str1, const char* str2, int count, CaseSensitivity cs);
template int StringHelper::compare<wchar_t>(const wchar_t* str1, const wchar_t* str2, int count, CaseSensitivity cs);
template int StringHelper::compare<char16_t>(const char16_t* str1, const char16_t* str2, int count, CaseSensitivity cs);

template<typename TChar>
int StringHelper::compare(TChar ch1, TChar ch2, CaseSensitivity cs)
{
	if (cs == CaseSensitivity::CaseSensitive) {
		return ((unsigned int)(ch1)) - ((unsigned int)(ch2));
	}
	else {
		return ((unsigned int)(toUpper(ch1))) - ((unsigned int)(toUpper(ch2)));
	}
}
template int StringHelper::compare<char>(char ch1, char ch2, CaseSensitivity cs);
template int StringHelper::compare<wchar_t>(wchar_t ch1, wchar_t ch2, CaseSensitivity cs);
template int StringHelper::compare<char16_t>(char16_t ch1, char16_t ch2, CaseSensitivity cs);

template<typename TChar>
void StringHelper::trim(const TChar* begin, int length, const TChar** outBegin, int* outLength)
{
	if (LN_ENSURE(begin != nullptr)) return;
	if (LN_ENSURE(length >= 0)) return;
	if (LN_ENSURE(outBegin != nullptr)) return;
	if (LN_ENSURE(outLength != nullptr)) return;

	if (length == 0) {
		*outBegin = begin;
		*outLength = 0;
		return;
	}

	const TChar* end = begin + length;

	// Left 部分
	while (*begin)
	{
		if (!isSpace(*begin)) {
			break;
		}
		++begin;
	}

	// Right 部分
	while (begin < end)
	{
		if (!isSpace(*(end - 1))) {
			break;
		}
		--end;
	}

	*outBegin = begin;
	*outLength = (int)(end - begin);
}
template void StringHelper::trim<char>(const char* begin, int length, const char** outBegin, int* outLength);
template void StringHelper::trim<wchar_t>(const wchar_t* begin, int length, const wchar_t** outBegin, int* outLength);
template void StringHelper::trim<char16_t>(const char16_t* begin, int length, const char16_t** outBegin, int* outLength);

template<typename TChar>
bool StringHelper::startsWith(const TChar* str1, int len1, const TChar* str2, int len2, CaseSensitivity cs)
{
	return compare(str1, str2, len2, cs) == 0;
}
template bool StringHelper::startsWith<char>(const char* str1, int len1, const char* str2, int len2, CaseSensitivity cs);
template bool StringHelper::startsWith<wchar_t>(const wchar_t* str1, int len1, const wchar_t* str2, int len2, CaseSensitivity cs);
template bool StringHelper::startsWith<char16_t>(const char16_t* str1, int len1, const char16_t* str2, int len2, CaseSensitivity cs);

template<typename TChar>
bool StringHelper::endsWith(const TChar* str1, int len1, const TChar* str2, int len2, CaseSensitivity cs)
{
	// 長さが -1 の場合は \0 までカウント
	len1 = static_cast<int>((len1 < 0) ? strlen(str1) : len1);
	len2 = static_cast<int>((len2 < 0) ? strlen(str2) : len2);

	const TChar* p1 = str1 + len1;
	const TChar* p2 = str2 + len2;

	// 大文字小文字を区別しない場合
	if (cs == CaseSensitivity::CaseInsensitive)
	{
		while (str2 <= p2)
		{
			if (p1 < str1 ||
				toupper(*p1) != toupper(*p2)) {
				return false;
			}
			p1--;
			p2--;
		}
		return true;
	}
	// 大文字小文字を区別する場合
	else
	{
		while (str2 <= p2)
		{
			if (p1 < str1 ||
				*p1 != *p2) {
				return false;
			}
			p1--;
			p2--;
		}
		return true;
	}
}
template bool StringHelper::endsWith<char>(const char* str1, int len1, const char* str2, int len2, CaseSensitivity cs);
template bool StringHelper::endsWith<wchar_t>(const wchar_t* str1, int len1, const wchar_t* str2, int len2, CaseSensitivity cs);
template bool StringHelper::endsWith<char16_t>(const char16_t* str1, int len1, const char16_t* str2, int len2, CaseSensitivity cs);

template<typename TChar>
void StringHelper::substr(const TChar* str, int len, int start, int count, const TChar** outBegin, const TChar** outEnd)
{
	if (start < 0)
	{
		start = 0;
	}
	if (count < 0)
	{
		count = len - start;
	}
	if (start + count > len)
	{
		count = len - start;
	}
	if (start > len)
	{
		count = 0;
	}

	if (start == 0 && count == len)
	{
		*outBegin = str;
		*outEnd = str + len;
	}
	else
	{
		*outBegin = str + start;
		*outEnd = str + start + count;
	}
}
template void StringHelper::substr<char>(const char* str, int len, int start, int count, const char** outBegin, const char** outEnd);
template void StringHelper::substr<wchar_t>(const wchar_t* str, int len, int start, int count, const wchar_t** outBegin, const wchar_t** outEnd);
template void StringHelper::substr<char16_t>(const char16_t* str, int len, int start, int count, const char16_t** outBegin, const char16_t** outEnd);

template<typename TChar>
void StringHelper::left(const TChar* str, int count, const TChar** outBegin, const TChar** outEnd)
{
	if (count < 0)
	{
		count = 0;
	}

	int len = (int)strlen(str);
	len = LN_MIN(len, count);
	*outBegin = str;
	*outEnd = str + len;
}
template void StringHelper::left<char>(const char* str, int count, const char** outBegin, const char** outEnd);
template void StringHelper::left<wchar_t>(const wchar_t* str, int count, const wchar_t** outBegin, const wchar_t** outEnd);
template void StringHelper::left<char16_t>(const char16_t* str, int count, const char16_t** outBegin, const char16_t** outEnd);

template<typename TChar>
void StringHelper::right(const TChar* str, int count, const TChar** outBegin, const TChar** outEnd)
{
	if (count < 0)
	{
		count = 0;
	}

	int len = (int)strlen(str);
	*outBegin = str + len - count;
	*outEnd = str + len;
}
template void StringHelper::right<char>(const char* str, int count, const char** outBegin, const char** outEnd);
template void StringHelper::right<wchar_t>(const wchar_t* str, int count, const wchar_t** outBegin, const wchar_t** outEnd);
template void StringHelper::right<char16_t>(const char16_t* str, int count, const char16_t** outBegin, const char16_t** outEnd);

//------------------------------------------------------------------------------
// Note:C言語標準では uint64_t 用の 変換関数が無いため自作した。
//		ちなみに C++ の istream も uint64_t 用の変換は標準ではない。(VC++にはあるが)
//------------------------------------------------------------------------------
template<typename TChar, typename TSigned, typename TUnigned>
static NumberConversionResult StrToNumInternal(
	const TChar* str,
	int len,			// -1 可
	int base,
	bool reqUnsigned,	// unsigned として読み取りたいかどうか
	TSigned signedMin,
	TSigned signedMax,
	TUnigned unsignedMax,
	const TChar** outEndPtr,	// NULL 可。処理完了後、str+len と一致しなかったら後ろに数字ではない文字がある。
	TUnigned* outNumber)		// NULL 可 (文字数カウント用)
{
	if (outNumber != NULL) { *outNumber = 0; }

	if (str == NULL) { return NumberConversionResult::ArgsError; }
	if (!(base == 0 || base == 2 || base == 8 || base == 10 || base == 16)) { return NumberConversionResult::ArgsError; }

	const TChar* p = str;
	const TChar* end = str + (len < 0 ? StringHelper::strlen(str) : len);

	// 空白をスキップ
	while (StringHelper::isSpace(*p)) { ++p; }

	// 符号チェック
	bool isNeg = false;
	if (*p == '-') {
		isNeg = true;	// マイナス符号有り
		++p;
	}
	else if (*p == '+') {
		++p;			// プラス符号はスキップするだけ
	}
	if (p >= end) { return NumberConversionResult::FormatError; }	// 符号しかなかった

																	// 基数が 0 の場合は自動判別する
	if (base == 0)
	{
		if (p[0] != '0') {
			base = 10;
		}
		else if (p[1] == 'x' || p[1] == 'X') {
			base = 16;
		}
		else {
			base = 8;
		}
	}

	// 基数によってプレフィックスをスキップする
	if (base == 8)
	{
		if (p[0] == '0')
		{
			++p;
			if (p >= end)
			{
				if (outEndPtr != NULL) { *outEndPtr = p; }
				*outNumber = 0;
				return NumberConversionResult::Success;
			}

		}
	}
	else if (base == 16)
	{
		if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
			++p;
			++p;
			if (p >= end) { return NumberConversionResult::FormatError; }	// 0x しかなかった
		}
	}

	TUnigned num = 0;
	TUnigned overflowMax = unsignedMax / base;	// 乗算しようとするとき、この値以上であれば次の乗算でオーバフローする
	TUnigned baseMax = unsignedMax % base;
	int count = 0;	// 読み取った文字数
	bool isOverflow = false;
	for (;;)
	{
		// 1文字読み取る
		TUnigned d;
		if (isdigit(*p)) {
			d = *p - '0';
		}
		else if ('A' <= *p && *p <= 'F') {
			d = *p - 'A' + 10;
		}
		else if ('a' <= *p && *p <= 'f') {
			d = *p - 'a' + 10;
		}
		else {
			break;
		}
		if (d >= (TUnigned)base) {
			// 基数より大きい桁が見つかった (10進数なのに A があったなど)
			return NumberConversionResult::FormatError;
		}
		++count;

		// 計算する前にオーバーフローを確認する
		if (num < overflowMax ||					// 例) uint8(max 255) のとき、num は 25 より小さければOK
			(num == overflowMax && d <= baseMax)) {	// 例) num が 25 のときは d が 5 以下であればOK
			num = num * base + d;
		}
		else {
			isOverflow = true;
		}

		// 次の文字へ
		++p;
		if (p >= end) { break; }
	}

	if (outEndPtr != NULL) {
		*outEndPtr = p;
	}

	// オーバーフローしていたら最大値に丸めておく
	NumberConversionResult result = NumberConversionResult::Success;
	if (isOverflow)
	{
		num = unsignedMax;
		result = NumberConversionResult::Overflow;
	}
	else if (!reqUnsigned)	// signed のオーバフローチェック
	{
		if (isNeg && num > (TUnigned)(-signedMin))
		{
			num = (TUnigned)(-signedMin);
			result = NumberConversionResult::Overflow;
		}
		else if (!isNeg && num > (TUnigned)signedMax)
		{
			num = signedMax;
			result = NumberConversionResult::Overflow;
		}
	}

	// マイナス符合がある場合は signed 値を unsigned として返し、
	// 呼び出し側で signed にキャストして返す。
	if (isNeg) {
		num = (TUnigned)(-(TSigned)num);
	}

	if (outNumber != NULL) { *outNumber = num; }
	return result;
}

template<typename TChar>
bool StringHelper::match(const TChar* pattern, const TChar* str)
{
	switch (*pattern)
	{
	case '\0':
		return '\0' == *str;
	case '*':
		return match(pattern + 1, str) || (('\0' != *str) && match(pattern, str + 1));
	case '?':
		return ('\0' != *str) && match(pattern + 1, str + 1);
	default:
		return (*pattern == *str) && match(pattern + 1, str + 1);
	}
}
template bool StringHelper::match<char>(const char* pattern, const char* str);
template bool StringHelper::match<wchar_t>(const wchar_t* pattern, const wchar_t* str);
template bool StringHelper::match<char16_t>(const char16_t* pattern, const char16_t* str);

template<typename TChar>
int8_t StringHelper::toInt8(const TChar* str, int len, int base, const TChar** outEndPtr, NumberConversionResult* outResult)
{
	uint8_t n;
	NumberConversionResult r = StrToNumInternal<TChar, int8_t, uint8_t>(str, len, base, false, INT8_MIN, INT8_MAX, UINT8_MAX, outEndPtr, &n);
	if (outResult != NULL) { *outResult = r; }
	return (int8_t)n;
}
template int8_t StringHelper::toInt8<char>(const char* str, int len, int base, const char** outEndPtr, NumberConversionResult* outResult);
template int8_t StringHelper::toInt8<wchar_t>(const wchar_t* str, int len, int base, const wchar_t** outEndPtr, NumberConversionResult* outResult);
template int8_t StringHelper::toInt8<char16_t>(const char16_t* str, int len, int base, const char16_t** outEndPtr, NumberConversionResult* outResult);

template<typename TChar>
uint8_t StringHelper::toUInt8(const TChar* str, int len, int base, const TChar** outEndPtr, NumberConversionResult* outResult)
{
	uint8_t n;
	NumberConversionResult r = StrToNumInternal<TChar, int8_t, uint8_t>(str, len, base, true, INT8_MIN, INT8_MAX, UINT8_MAX, outEndPtr, &n);
	if (outResult != NULL) { *outResult = r; }
	return n;
}
template uint8_t StringHelper::toUInt8<char>(const char* str, int len, int base, const char** outEndPtr, NumberConversionResult* outResult);
template uint8_t StringHelper::toUInt8<wchar_t>(const wchar_t* str, int len, int base, const wchar_t** outEndPtr, NumberConversionResult* outResult);
template uint8_t StringHelper::toUInt8<char16_t>(const char16_t* str, int len, int base, const char16_t** outEndPtr, NumberConversionResult* outResult);

template<typename TChar>
int16_t StringHelper::toInt16(const TChar* str, int len, int base, const TChar** outEndPtr, NumberConversionResult* outResult)
{
	uint16_t n;
	NumberConversionResult r = StrToNumInternal<TChar, int16_t, uint16_t>(str, len, base, false, INT16_MIN, INT16_MAX, UINT16_MAX, outEndPtr, &n);
	if (outResult != NULL) { *outResult = r; }
	return (int16_t)n;
}
template int16_t StringHelper::toInt16<char>(const char* str, int len, int base, const char** outEndPtr, NumberConversionResult* outResult);
template int16_t StringHelper::toInt16<wchar_t>(const wchar_t* str, int len, int base, const wchar_t** outEndPtr, NumberConversionResult* outResult);
template int16_t StringHelper::toInt16<char16_t>(const char16_t* str, int len, int base, const char16_t** outEndPtr, NumberConversionResult* outResult);

template<typename TChar>
uint16_t StringHelper::toUInt16(const TChar* str, int len, int base, const TChar** outEndPtr, NumberConversionResult* outResult)
{
	uint16_t n;
	NumberConversionResult r = StrToNumInternal<TChar, int16_t, uint16_t>(str, len, base, true, INT16_MIN, INT16_MAX, UINT16_MAX, outEndPtr, &n);
	if (outResult != NULL) { *outResult = r; }
	return n;
}
template uint16_t StringHelper::toUInt16<char>(const char* str, int len, int base, const char** outEndPtr, NumberConversionResult* outResult);
template uint16_t StringHelper::toUInt16<wchar_t>(const wchar_t* str, int len, int base, const wchar_t** outEndPtr, NumberConversionResult* outResult);
template uint16_t StringHelper::toUInt16<char16_t>(const char16_t* str, int len, int base, const char16_t** outEndPtr, NumberConversionResult* outResult);

template<typename TChar>
int32_t StringHelper::toInt32(const TChar* str, int len, int base, const TChar** outEndPtr, NumberConversionResult* outResult)
{
	uint32_t n;
	NumberConversionResult r = StrToNumInternal<TChar, int32_t, uint32_t>(str, len, base, false, INT32_MIN, INT32_MAX, UINT32_MAX, outEndPtr, &n);
	if (outResult != NULL) { *outResult = r; }
	return (int32_t)n;
}
template int32_t StringHelper::toInt32<char>(const char* str, int len, int base, const char** outEndPtr, NumberConversionResult* outResult);
template int32_t StringHelper::toInt32<wchar_t>(const wchar_t* str, int len, int base, const wchar_t** outEndPtr, NumberConversionResult* outResult);
template int32_t StringHelper::toInt32<char16_t>(const char16_t* str, int len, int base, const char16_t** outEndPtr, NumberConversionResult* outResult);

template<typename TChar>
uint32_t StringHelper::toUInt32(const TChar* str, int len, int base, const TChar** outEndPtr, NumberConversionResult* outResult)
{
	uint32_t n;
	NumberConversionResult r = StrToNumInternal<TChar, int32_t, uint32_t>(str, len, base, true, INT32_MIN, INT32_MAX, UINT32_MAX, outEndPtr, &n);
	if (outResult != NULL) { *outResult = r; }
	return n;
}
template uint32_t StringHelper::toUInt32<char>(const char* str, int len, int base, const char** outEndPtr, NumberConversionResult* outResult);
template uint32_t StringHelper::toUInt32<wchar_t>(const wchar_t* str, int len, int base, const wchar_t** outEndPtr, NumberConversionResult* outResult);
template uint32_t StringHelper::toUInt32<char16_t>(const char16_t* str, int len, int base, const char16_t** outEndPtr, NumberConversionResult* outResult);

template<typename TChar>
int64_t StringHelper::toInt64(const TChar* str, int len, int base, const TChar** outEndPtr, NumberConversionResult* outResult)
{
	uint64_t n;
	NumberConversionResult r = StrToNumInternal<TChar, int64_t, uint64_t>(str, len, base, false, INT64_MIN, INT64_MAX, UINT64_MAX, outEndPtr, &n);
	if (outResult != NULL) { *outResult = r; }
	return (int64_t)n;
}
template int64_t StringHelper::toInt64<char>(const char* str, int len, int base, const char** outEndPtr, NumberConversionResult* outResult);
template int64_t StringHelper::toInt64<wchar_t>(const wchar_t* str, int len, int base, const wchar_t** outEndPtr, NumberConversionResult* outResult);
template int64_t StringHelper::toInt64<char16_t>(const char16_t* str, int len, int base, const char16_t** outEndPtr, NumberConversionResult* outResult);

template<typename TChar>
uint64_t StringHelper::toUInt64(const TChar* str, int len, int base, const TChar** outEndPtr, NumberConversionResult* outResult)
{
	uint64_t n;
	NumberConversionResult r = StrToNumInternal<TChar, int64_t, uint64_t>(str, len, base, true, INT64_MIN, INT64_MAX, UINT64_MAX, outEndPtr, &n);
	if (outResult != NULL) { *outResult = r; }
	return n;
}
template uint64_t StringHelper::toUInt64<char>(const char* str, int len, int base, const char** outEndPtr, NumberConversionResult* outResult);
template uint64_t StringHelper::toUInt64<wchar_t>(const wchar_t* str, int len, int base, const wchar_t** outEndPtr, NumberConversionResult* outResult);
template uint64_t StringHelper::toUInt64<char16_t>(const char16_t* str, int len, int base, const char16_t** outEndPtr, NumberConversionResult* outResult);

#ifdef _WIN32
static bool g_localeInitialized = false;
static _locale_t g_locale;

static _locale_t GetCLocale()
{
	if (!g_localeInitialized)
	{
		g_locale = _create_locale(LC_ALL, "C");
		g_localeInitialized = true;
	}
	return g_locale;
}

static double StrToD_L(const char* str, char** endptr, _locale_t locale) { return _strtod_l(str, endptr, locale); }
static double StrToD_L(const wchar_t* str, wchar_t** endptr, _locale_t locale) { return _wcstod_l(str, endptr, locale); }

#else
static bool g_localeInitialized = false;

class CLocale
{
public:
	locale_t m_locale;
	CLocale() : m_locale(0) { }
	~CLocale()
	{
		if (m_locale != NULL) {
			freelocale(m_locale);
		}
	}
};
static CLocale g_cLocale;

static locale_t GetCLocale()
{
	if (!g_localeInitialized)
	{
		g_cLocale.m_locale = newlocale(LC_ALL_MASK, "C", NULL);
		g_localeInitialized = true;
	}
	return g_cLocale.m_locale;
}
static double StrToD_L(const char* str, char** endptr, locale_t locale) { return strtod_l(str, endptr, locale); }
//static double StrToD_L(const wchar_t* str, wchar_t** endptr, locale_t locale) { return wcstod_l(str, endptr, locale); }

#endif

template<typename TChar>
double StringHelper::toDouble(const TChar* str, int len, const TChar** outEndPtr, NumberConversionResult* outResult)
{
	if (outResult != NULL) { *outResult = NumberConversionResult::Success; }

	if (str == NULL) {
		if (outResult != NULL) { *outResult = NumberConversionResult::ArgsError; }
		return 0.0;
	}

	len = static_cast<int>((len < 0 ? StringHelper::strlen(str) : len));
	if (len >= 512) {
		if (outResult != NULL) { *outResult = NumberConversionResult::ArgsError; }
		return 0.0;
	}

	// 標準関数の strtod は長さを渡せないので一時バッファにコピーして終端\0にする。
	// 最大長さはとりあえず 512。
	// IEEE 形式では仮数部の桁数は 2^53=9007199254740992 で16桁で、指数部は 308。
	// IBM 形式では仮数部の桁数は 2^24=16777216 で8桁で、指数部は 16^63で、7.237005577332262213973186563043e+75。
	// 0 は 308 個並べられることになるが、512 文字分のサイズがあれば十分。
	char tmp[512] = { 0 };
	copySimpleAsciiString(tmp, 512, str, len);
	tmp[len] = '\0';

	char* end;
	errno = 0;
	double v = StrToD_L(tmp, &end, GetCLocale());

	if (outEndPtr != NULL) { *outEndPtr = str + (end - tmp); }

	if (errno == ERANGE || v == HUGE_VAL || v == -HUGE_VAL) {
		if (outResult != NULL) { *outResult = NumberConversionResult::Overflow; }
		return v;
	}
	return v;
}
template double StringHelper::toDouble<char>(const char* str, int len, const char** outEndPtr, NumberConversionResult* outResult);
template double StringHelper::toDouble<wchar_t>(const wchar_t* str, int len, const wchar_t** outEndPtr, NumberConversionResult* outResult);
template double StringHelper::toDouble<char16_t>(const char16_t* str, int len, const char16_t** outEndPtr, NumberConversionResult* outResult);

template<typename TChar>
float StringHelper::toFloat(const TChar* str, int len, const TChar** outEndPtr, NumberConversionResult* outResult)
{
	double value = toDouble(str, len, outEndPtr, outResult);

	// double に変換するとオーバーフローする場合はエラーを出力する
	if (outResult != nullptr &&
		*outResult == NumberConversionResult::Success &&
		-FLT_MAX <= value && value <= FLT_MAX)
	{
		*outResult = NumberConversionResult::Overflow;
	}

	return (float)value;
}
template float StringHelper::toFloat<char>(const char* str, int len, const char** outEndPtr, NumberConversionResult* outResult);
template float StringHelper::toFloat<wchar_t>(const wchar_t* str, int len, const wchar_t** outEndPtr, NumberConversionResult* outResult);
template float StringHelper::toFloat<char16_t>(const char16_t* str, int len, const char16_t** outEndPtr, NumberConversionResult* outResult);

int StringHelper::int64ToString(int64_t value, char format, char* outStr, int bufSize)
{
	ln::detail::StdCharArrayBuffer<char> b(outStr, bufSize);
	std::basic_ostream<char, std::char_traits<char> > os(&b);
	//os.imbue(*formatter.m_locale);

	if (format == 'd' || format == 'D')
	{
	}
	else if (format == 'x' || format == 'X')
	{
		os << std::hex;
		if (format == 'X') os << std::uppercase;
	}

	os << value;
	return static_cast<int>(b.length());
}

int StringHelper::uint64ToString(uint64_t value, char format, char* outStr, int bufSize)
{
	ln::detail::StdCharArrayBuffer<char> b(outStr, bufSize);
	std::basic_ostream<char, std::char_traits<char> > os(&b);
	//os.imbue(*formatter.m_locale);

	if (format == 'd' || format == 'D')
	{
	}
	else if (format == 'x' || format == 'X')
	{
		os << std::hex;
		if (format == 'X') os << std::uppercase;
	}

	os << value;
	return static_cast<int>(b.length());
}

int StringHelper::doubleToString(double value, char format, int precision, char* outStr, int bufSize)
{
	ln::detail::StdCharArrayBuffer<char> b(outStr, bufSize);
	std::basic_ostream<char, std::char_traits<char> > os(&b);
	//os.imbue(*formatter.m_locale);

	if (format == 'f' || format == 'F')
	{
		os << std::fixed;
	}
	else if (format == 'e' || format == 'E')
	{
		os << std::scientific;
		if (format == 'E') os << std::uppercase;
	}
	if (precision >= 0)
	{
		os << std::setprecision(precision);
	}

	os << value;
	return static_cast<int>(b.length());
}

//==============================================================================
// StringHelper

namespace detail {

size_t UStringHelper::strlen(const Char* str)
{
	if (!str) return 0;
	size_t count = 0;
	for (; *str; ++str) ++count;
	return count;
}

int UStringHelper::compare(const Char* str1, const Char* str2)
{
	str1 = (str1) ? str1 : _TT("");
	str2 = (str2) ? str2 : _TT("");
	for (; *str1 && *str2; ++str1, ++str2)
	{
		if (*str1 != *str2)
		{
			return (*str1 < *str2) ? -1 : 1;
		}
	}

	if (*str1 != *str2)
		return (*str1 < *str2) ? -1 : 1;
	else
		return 0;
}

template<typename TChar, typename TValue>
static void toStringIntX(TValue v, TChar* outStr, int size)
{
	char buf[64];
	detail::StdCharArrayBuffer<char> b(buf, 64);
	std::ostream os(&b);
	os << v;
	const char* str = b.GetCStr();
	int i = 0;
	for (; *str && i < (size - 1); ++str, ++i)
	{
		outStr[i] = *str;
	}
	outStr[i] = '\0';
}

template<typename TChar>
void UStringHelper::toStringInt8(int8_t v, TChar* outStr, int size)
{
	toStringIntX((int32_t)v, outStr, size);
}
template void UStringHelper::toStringInt8<char>(int8_t v, char* outStr, int size);
template void UStringHelper::toStringInt8<wchar_t>(int8_t v, wchar_t* outStr, int size);
template void UStringHelper::toStringInt8<char16_t>(int8_t v, char16_t* outStr, int size);


} // namespace detail
} // namespace ln
