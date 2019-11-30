﻿#pragma once

#if !defined(LN_FLAT_API)
    #if defined(_WIN32)
        #define LN_FLAT_API __declspec(dllexport)
    #elif defined(_WIN32) && defined(LUMINO_DLL)
        #define LN_FLAT_API __declspec(dllimport)
    #elif defined(__GNUC__) && defined(LUMINO_BUILD_DLL)
        #define LN_FLAT_API __attribute__((visibility("default")))
    #else
        #define LN_FLAT_API
    #endif
#endif


#define LNI_FUNC_TRY_BEGIN    try {
#define LNI_FUNC_TRY_END_RETURN    } \
    catch (ln::Exception& e) { \
        return ln::Runtime::processException(&e); \
    } \
    catch (...) { \
        return LN_ERROR_UNKNOWN; \
    } \
	return LN_SUCCESS;

#define LNI_BOOL_TO_LNBOOL(x)    (x) ? LN_TRUE : LN_FALSE
#define LNI_LNBOOL_TO_BOOL(x)    (x != LN_FALSE)

#define LNI_CREATE_OBJECT(out, type, initFunc, ...) { auto ptr = ::ln::makeObject<type>(__VA_ARGS__); ::ln::RefObjectHelper::retain(ptr); *out = ::ln::Runtime::makeObjectWrap(ptr, true); }
#define LNI_HANDLE_TO_OBJECT(type, h)               static_cast<type*>((h) ? ::ln::Runtime::getObject(h) : nullptr)
#define LNI_OBJECT_TO_HANDLE(obj)					::ln::Runtime::makeObjectWrap(obj, false)

#define LNI_REFERENCE_RETAINED (1)
#define LNI_REFERENCE_RELEASED (2)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//------------------------------------------------------------------------------
#include <stdint.h>
typedef intptr_t LnHandle;
typedef void* LnUserData;
typedef char16_t LnChar;

#define LN_NULL_HANDLE 0

/** 結果・エラーコード */
typedef enum tagLnResult
{
    /** 成功 */
	LN_SUCCESS = 0,

    /** 不明なエラー */
    LN_ERROR_UNKNOWN = -1,

} LnResult;

/** 真偽値 */
typedef enum tagLnBool
{
    /** 偽 */
    LN_FALSE = 0,

    /** 真 */
    LN_TRUE = 1,

} LnBool;


typedef void(*LnReferenceCountTrackerCallback)(LnHandle handle, int method, int count);
typedef void(*LnRuntimeFinalizedCallback)();

inline const char* LnRuntime_GetLastErrorMessage() { return ""; }  // TODO:
extern LN_FLAT_API void LnRuntime_SetManagedObjectId(LnHandle handle, int64_t id);
extern LN_FLAT_API int64_t LnRuntime_GetManagedObjectId(LnHandle handle);
extern LN_FLAT_API int64_t LnRuntime_GetManagedTypeInfoId(LnHandle handle);
extern LN_FLAT_API void LnRuntime_SetReferenceCountTracker(LnReferenceCountTrackerCallback callback);
extern LN_FLAT_API void LnRuntime_SetReferenceTrackEnabled(LnHandle handle);
extern LN_FLAT_API void LnRuntime_SetRuntimeFinalizedCallback(LnRuntimeFinalizedCallback callback);

//==============================================================================

/**
	@brief	全てのオブジェクトのベースクラスです。
*/

/**
	@brief		オブジェクトを解放します。
	@param[in]	obj	: オブジェクトハンドル
	@details	指定されたオブジェクトの参照を解放します。
*/
LN_FLAT_API LnResult LnObject_Release(LnHandle obj);

/**
 *  @brief      オブジェクトの参照を取得します。
 *  @param[in]  obj	: オブジェクトハンドル
 *  @details    指定されたオブジェクトの参照カウントをインクリメントします。
 */
LN_FLAT_API LnResult LnObject_Retain(LnHandle obj);

/**
 *  @brief      ネイティブオブジェクトの参照カウントを取得します。これは内部的に使用される関数です。
 *  @param[in]  obj	: オブジェクトハンドル
 */
LN_FLAT_API int32_t LnObject_GetReferenceCount(LnHandle obj);


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

