﻿#pragma once

#include <LuminoEngine.hpp>
#include <LuminoEngine/Runtime/Lumino.FlatC.generated.h>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "hsp3plugin/hsp3plugin.h"

#define sbAlloc hspmalloc
#define sbFree hspfree

extern bool g_leadSupport;

void* hspCommon_GetBlockSize(PVal* pval, PDAT* pdat, int* size);
void hspCommon_AllocBlock(PVal* pval, PDAT* pdat, int size);

bool checkAndFetchDefault();
double fetchVADouble_reffunc();

int fetchVAInt();
int fetchVAInt(int defaultValue);
double fetchVADouble();
double fetchVADouble(double defaultValue);
const char* fetchVAString();
const char* fetchVAString(const char* defaultValue);
void* fetchVARawData();
unsigned short* fetchVALabelPointer();

void setVAInt(PVal* pval, APTR aptr, int value);
void setVADouble(PVal* pval, APTR aptr, double value);
void setVAStr(PVal* pval, APTR aptr, const std::string& value);

#define CodeGetVA_TypeChecked(ppval, type) \
	code_getva(ppval); \
	if ((*ppval)->flag != hsp##type##_typeid()) { throw HSPVAR_ERROR_TYPEMISS; }


static const int MaxArgs = 16;
extern std::array<ln::Ref<ln::Variant>, MaxArgs> g_callbackArgs;

template<class T>
void setCallbackArg(int index, T && value)
{
	g_callbackArgs[index] = ln::makeVariant(std::forward<T>(value));
}

template<class T>
void setCallbackOutput(int index, T* value)
{
	LN_NOTIMPLEMENTED();
}

void ln_args_reffunc(int* typeRes, void** retValPtr);

