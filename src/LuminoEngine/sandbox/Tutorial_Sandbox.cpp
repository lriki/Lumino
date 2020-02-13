﻿
#include <LuminoEngine.hpp>
#include <LuminoEngine/UI/UIComboBox.hpp>
using namespace ln;

class App : public Application
{
	Ref<BoxMesh> box;
	float posX = 0;
	float posY = 0;

	virtual void onInit() override
	{
		box = BoxMesh::create();
	}

	virtual void onUpdate() override
	{
		if (Input::isPressed(u"left")) {
			posX -= 0.1;
		}

		if (Input::isPressed(u"right")) {
			posX += 0.1;
		}

		if (Input::isPressed(u"up")) {
			posY += 0.1;
		}

		if (Input::isPressed(u"down")) {
			posY -= 0.1;
		}

		if (Input::isTriggered(u"submit")) {
			posX = 0;
			posY = 0;
		}

		box->setPosition(posX, posY, 0);
	}
};

//class App : public Application
//{
//	Ref<SphereMesh> s;
//
//	virtual void onInit() override
//	{
//		//Engine::renderView()->setGuideGridEnabled(true);
//		Engine::camera()->addComponent(CameraOrbitControlComponent::create());
//
//		//auto texture = Texture2D::load(u"C:/Proj/LN/Lumino/src/LuminoEngine/sandbox/Assets/picture1.jpg");
//		//auto sprite = Sprite::create(texture, -1, 6);
//		auto sprite = Sprite::load(u"C:/Proj/LN/Lumino/src/LuminoEngine/sandbox/Assets/picture1.jpg");
//		sprite->setSize(Size(-1, 2));
//
//		s = SphereMesh::create();
//		s->setShadingModel(ShadingModel::UnLighting);
//		s->setColorScale(Color::Yellow);
//	}
//
//	virtual void onUpdate() override
//	{
//		auto light = Engine::light();
//
//		
//		s->setPosition(-light->transform()->getFront() * 5);;
//
//		//float x, y;
//		//Math::sinCos(Engine::time(), &x, &y);
//		//light->lookAt(x, 0, y);
//
//		//s->setPosition(-Vector3(x, 0, y) * 5);
//	}
//};

void Tutorial_Sandbox()
{
    App app;
    EngineSettings::setDebugToolEnabled(false);
    detail::ApplicationHelper::run(&app);
}




