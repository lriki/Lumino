﻿
#pragma once
#include "../Graphics/ColorStructs.hpp"
#include "../Graphics/GeometryStructs.hpp"

namespace ln {
class Level;
class Camera;
class Raycaster;
class RaycastResult;
class Material;
class ReflectorComponent;

enum class LevelTransitionEffectMode
{
	None,

	/**  */
	FadeInOut,

	/**  */
	CrossFade,
};

} // namespace ln
