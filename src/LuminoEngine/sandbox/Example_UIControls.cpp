﻿
#include <LuminoEngine.hpp>
#include <LuminoEngine/UI/UIComboBox.hpp>
using namespace ln;

class App_Example_UIControls : public Application
{

    virtual void onInit() override
    {
        Engine::mainCamera()->setBackgroundColor(Color::Gray);

        auto combobox1 = ln::makeObject<UIComboBox>();
        combobox1->setWidth(200);
        combobox1->setHeight(32);
        combobox1->setBackgroundColor(Color::BurlyWood);
        Engine::mainUIView()->addElement(combobox1);
    }

    virtual void onUpdate() override
    {
    }
};

void Example_UIControls()
{
    App_Example_UIControls app;
	detail::ApplicationHelper::init(&app);
	detail::ApplicationHelper::run(&app);
}



