#include "platform/pch.h"
#include <Windows.h>
#include "Application.h"

#include <iostream>

// External function - the actual main loop logic stays in capsicain.cpp for now
extern int capsicain_main_impl();

Application::Application()
    : interceptionContext_(nullptr)
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
    // Delegate to existing main() implementation
    // This preserves all existing logic while creating the Application structure
    return capsicain_main_impl();
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
