﻿#include "Common.hpp"
#include <LuminoEngine/Runtime/Lumino.FlatC.generated.h>

//==============================================================================
class Test_FlatAPI : public LuminoSceneTest {};

//------------------------------------------------------------------------------
static int g_count = 0;

static LnResult Sprite_OnUpdate(LnHandle worldobject, float elapsedSeconds)
{
	g_count++;
	return LN_SUCCESS;
}

TEST_F(Test_FlatAPI, Basic)
{
	LnHandle texture;
	LnTexture2D_Load(LN_ASSETFILE("Sprite1.png"), &texture);

	LnHandle sprite;
	LnSprite_CreateWithTexture(texture, &sprite);
	LnWorldObject_SetScaleS(sprite, 5);
	LnSprite_SetPrototype_OnUpdate(sprite, Sprite_OnUpdate);

	g_count = 0;

	TestEnv::updateFrame();
	ASSERT_SCREEN(LN_ASSETFILE("FlatAPI/Expects/Basic-1.png"));
	LN_TEST_CLEAN_SCENE;

	LnObject_Release(sprite);
	LnObject_Release(texture);

	ASSERT_EQ(g_count, 1);
}

