﻿
#include <LuminoEngine.hpp>
#include <LuminoEngine/Scripting/Interpreter.hpp>
using namespace ln;

class App_Experiment_CharacterController : public Application
{
	

    void onInit() override
    {
		Engine::renderView()->setGuideGridEnabled(true);
		Engine::renderView()->setPhysicsDebugDrawEnabled(true);


		auto box = ln::makeObject<ln::BoxMesh>();
		//addComponent(box);

		auto controller = ln::makeObject<ln::CharacterController>();
		box->addComponent(controller);

		auto shape1 = BoxCollisionShape::create(2, 2, 2);
		auto body1 = TriggerBodyComponent::create();
		body1->addCollisionShape(shape1);
		box->addComponent(body1);


		auto box2 = ln::makeObject<ln::BoxMesh>();
		auto shape2 = BoxCollisionShape::create(2, 1, 1);
		auto body2 = TriggerBodyComponent::create();
		body2->addCollisionShape(shape2);
		box2->addComponent(body2);
	}

    void onUpdate() override
    {
    }

	bool onMessage(InterpreterCommandList* cmd)
	{

	}
};

void Experiment_CharacterController()
{
	App_Experiment_CharacterController app;
	detail::ApplicationHelper::run(&app);
}




