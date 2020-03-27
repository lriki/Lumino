﻿
#include <LuminoEngine.hpp>
#include <LuminoEngine/UI/UIComboBox.hpp>
#include <LuminoEngine/Scene/MeshTilemap/MeshTileset.hpp>
#include <LuminoEngine/Scene/MeshTilemap/MeshTilemapLayer.hpp>
#include <LuminoEngine/Scene/MeshTilemap/MeshTilemapModel.hpp>
#include <LuminoEngine/Scene/MeshTilemap/MeshTilemapComponent.hpp>
#include <LuminoEngine/ImageEffect/TransitionImageEffect.hpp>
using namespace ln;

TransitionImageEffect* g_TransitionImageEffect;

class TitleScene : public Level
{
	void onStart() override
	{
		//auto sprite = Sprite::create(Texture2D::load(u"picture1"));

		Debug::print(u"TitleScene");
		Engine::renderView()->setBackgroundColor(Color::White);
		//g_TransitionImageEffect->start(5);
		Scene::setTransitionMode(LevelTransitionMode::CrossFade);
		Scene::setTransitionEffectColor(Color::Red);
		//Scene::setTransitionEffectMaskTexture(Texture2D::load(u"Transition1"));
		//Scene::setTransitionEffectVague(0);
		Scene::setTransitionEffectMaskTexture(Texture2D::load(u"Transition2"));
	}

	void onUpdate() override;
};

class GameScene : public Level
{
	void onStart() override
	{
		Debug::print(u"GameScene");
		Engine::renderView()->setBackgroundColor(Color::Gray);
		//g_TransitionImageEffect->startFadeOut(5);
	}

	void onUpdate() override
	{
		if (!Scene::isTransitionEffectRunning() && Input::triggered(u"submit")) {
			Scene::gotoLevel(makeObject<TitleScene>());
		}
	}
};

void TitleScene::onUpdate()
{
	if (!Scene::isTransitionEffectRunning() && Input::triggered(u"submit")) {
		Scene::gotoLevel(makeObject<GameScene>());
	}
}

class App : public Application
{
	void onInit() override
	{
		Engine::renderView()->setGuideGridEnabled(true);
		Engine::camera()->addComponent(CameraOrbitControlComponent::create());


		//auto transitionImageEffect = TransitionImageEffect::create();
	 //   Engine::renderView()->addImageEffect(transitionImageEffect);
		//g_TransitionImageEffect = transitionImageEffect;

		Engine::world()->gotoScene(makeObject<TitleScene>());
	}

	void onUpdate() override
	{
	}
};

void Tutorial_Sandbox()
{
    App app;
	//EngineSettings::setFrameRate(10);
    detail::ApplicationHelper::run(&app);
}




