﻿
#pragma once
#include <functional>
#include "Common.h"
#include "EasingFunctions.h"

namespace Lumino
{

class AnimationUtilities
{
public:

	typedef std::function< float(float, float, float, float) >	FloatEasingFunction;

	template<typename TValue, typename TTime>
	static std::function< TValue(TTime, TValue, TValue, TTime) > SelectEasingFunction(Animation::EasingMode mode)
	{
		typedef TValue(*EasingFunctionPtr)(TTime, TValue, TValue, TTime);
		EasingFunctionPtr table[] =
		{
			EasingFunctions::LinearTween,
			EasingFunctions::EaseInQuad,
			EasingFunctions::EaseOutQuad,
			EasingFunctions::EaseInOutQuad,
			EasingFunctions::EaseInCubic,
			EasingFunctions::EaseOutCubic,
			EasingFunctions::EaseInOutCubic,
			EasingFunctions::EaseInQuart,
			EasingFunctions::EaseOutQuart,
			EasingFunctions::EaseInOutQuart,
			EasingFunctions::EaseInQuint,
			EasingFunctions::EaseOutQuint,
			EasingFunctions::EaseInOutQuint,
			EasingFunctions::EaseInSine,
			EasingFunctions::EaseOutSine,
			EasingFunctions::EaseInOutSine,
			EasingFunctions::EaseInExpo,
			EasingFunctions::EaseOutExpo,
			EasingFunctions::EaseInOutExpo,
			EasingFunctions::EaseInCirc,
			EasingFunctions::EaseOutCirc,
			EasingFunctions::EaseInOutCirc,
		};
		assert(LN_ARRAY_SIZE_OF(table) == Animation::EasingMode::GetMemberCount());
		return table[mode];
	}


};

} // namespace Lumino
