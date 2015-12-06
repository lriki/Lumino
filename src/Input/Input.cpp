
#include "Internal.h"
#include "InputManager.h"
#include "VirtualPad.h"
#include <Lumino/Input/Input.h>

LN_NAMESPACE_BEGIN

//=============================================================================
// Input
//=============================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool Input::IsPress(const TCHAR* bindingName)
{
	return detail::GetInputManager(nullptr)->GetVirtualPad(0)->IsPress(bindingName);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool Input::IsOnTrigger(const TCHAR* bindingName)
{
	return detail::GetInputManager(nullptr)->GetVirtualPad(0)->IsOnTrigger(bindingName);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool Input::IsOffTrigger(const TCHAR* bindingName)
{
	return detail::GetInputManager(nullptr)->GetVirtualPad(0)->IsOffTrigger(bindingName);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool Input::IsRepeat(const TCHAR* bindingName)
{
	return detail::GetInputManager(nullptr)->GetVirtualPad(0)->IsRepeat(bindingName);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
float Input::GetAxisValue(const TCHAR* bindingName)
{
	return detail::GetInputManager(nullptr)->GetVirtualPad(0)->GetAxisValue(bindingName);
}

//=============================================================================
// InputButton
//=============================================================================
const String& InputButtons::Left = _T("Left");
const String& InputButtons::Right = _T("Right");
const String& InputButtons::Up = _T("Up");
const String& InputButtons::Down = _T("Down");

LN_NAMESPACE_END
