﻿// Copyright (c) 2018+ lriki. Distributed under the MIT license..
#pragma once

namespace ln {
class Archive;

enum class ArchiveMode
{
	Save,
	Load,
};

enum class SerializeClassFormat
{
	Default,
	String,
};

namespace detail
{

struct NameValuePairBase {};

} // namespace detail

template<typename TValue>
struct NameValuePair : public detail::NameValuePairBase
{
public:
	const StringRef& name;
	TValue* value;
	const TValue* defaultValue;

	NameValuePair(const StringRef& n, TValue* v) : name(n), value(v), defaultValue(nullptr) {}
	NameValuePair(const StringRef& n, TValue* v, const TValue& defaultValue) : name(n), value(v), defaultValue(&defaultValue) {}

	bool hasDefaultValue() const { return defaultValue != nullptr; }

private:
	NameValuePair & operator=(NameValuePair const &) = delete;
};



namespace detail
{

// void serialize(Archive& ar) をメンバ関数として持っているか
template<typename T>
class has_member_serialize_function
{
private:
	template<typename U>
	static auto check(U&& v) -> decltype(v.serialize(*reinterpret_cast<Archive*>(0)), std::true_type());
	static auto check(...) -> decltype(std::false_type());

public:
	typedef decltype(check(std::declval<T>())) type;
	static bool const value = type::value;
};

// void serialize(Archive& ar) をメンバ関数として持っていないか
template<typename T>
class non_member_serialize_function
{
private:
	template<typename U>
	static auto check(U&& v) -> decltype(v.serialize(*reinterpret_cast<Archive*>(0)), std::true_type());
	static auto check(...) -> decltype(std::false_type());

public:
	typedef decltype(check(std::declval<T>())) type;
	static bool const value = !type::value;
};

// lumino_class_version をメンバとして持っているか
template<typename T>
class has_static_member_class_version
{
private:
	template<typename U>
	static auto check(U&& v) -> decltype(U::lumino_class_version, std::true_type());
	static auto check(...) -> decltype(std::false_type());

public:
	typedef decltype(check(std::declval<T>())) type;
	static bool const value = type::value;
};

template <class T> struct SerializeClassVersionInfo
{
	static const int value = 0;
};

template <class T> struct SerializeClassFormatInfo
{
	static const SerializeClassFormat value = SerializeClassFormat::Default;
};

template<class T>
struct ArchiveValueTraits
{
	static bool isPrimitiveType() { return false; }
};

template<>
struct ArchiveValueTraits<bool>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<int8_t>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<int16_t>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<int32_t>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<int64_t>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<uint8_t>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<uint16_t>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<uint32_t>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<uint64_t>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<float>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<double>
{
	static bool isPrimitiveType() { return true; }
};

template<>
struct ArchiveValueTraits<String>
{
	static bool isPrimitiveType() { return true; }
};

} // namespace detail
} // namespace ln
