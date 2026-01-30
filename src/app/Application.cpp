#include "platform/pch.h"
#include <Windows.h>
#include "Application.h"

#include <iostream>

// External function - the actual main loop logic stays in capsicain.cpp for now
extern int capsicain_main_impl(Application* app);

Application::Application()
    : uiService_(consoleUI_)
    , interceptionContext_(nullptr)
    , exitRequested_(false)
{
}

Application::~Application()
{
    shutdown();
}

bool Application::initialize()
{
    // Initialization is handled in capsicain_main_impl() for now
    // This keeps changes minimal and maintains existing behavior
    return true;
}

int Application::run()
{
    // Initialize services
    uiService_.initialize();
    configService_.loadIniFile();

    // Delegate to existing main() implementation
    // This preserves all existing logic while creating the Application structure
    return capsicain_main_impl(this);
}

void Application::shutdown()
{
    // Cleanup handled in capsicain_main_impl() for now
}

bool Application::initConsole()
{
    return true;
}

bool Application::initInterception()
{
    return true;
}

bool Application::loadConfiguration()
{
    return true;
}
