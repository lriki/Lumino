﻿
#include <LuminoEngine.hpp>
#include <LuminoEngine/UI/UIComboBox.hpp>
using namespace ln;

class App : public Application
{
	virtual void onInit() override
	{
		//Camera* camera = Engine::camera();
		//camera->addComponent(makeObject<CameraOrbitControlComponent>());
		//Engine::camera()->setPosition(10, 10, -10);
		//Engine::camera()->lookAt(0, 0, 0);
		//Engine::mainRenderView()->setDebugGridEnabled(false);

		//auto box = BoxMesh::create();

		auto camera = Engine::camera();
		camera->setPosition(5, 5, -5);
		camera->lookAt(0, 0, 0);
		camera->addComponent(makeObject<CameraOrbitControlComponent>());


		Engine::light()->lookAt(0, -1, 0);

		//auto light = DirectionalLight::create();


		auto sphere = SphereMesh::create();
	}

	virtual void onUpdate() override
	{
		//Debug::print(0, String::format(u"X: {0}", Mouse::position().x));
		//Debug::print(0, String::format(u"Y: {0}", Mouse::position().y));
	}
};

void Tutorial_Sandbox()
{
    App app;
	EngineSettings::setDebugToolEnabled(false);
	detail::ApplicationHelper::run(&app);
}



