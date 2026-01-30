/**
 * main.cpp
 *
 * Entry point for Capsicain - Advanced keyboard remapping tool
 *
 * This is a learning project maintained with AI assistance (Claude).
 * The codebase is being refactored from a monolithic design to a
 * clean domain-driven architecture as a C++ learning exercise.
 */

#include "app/Application.h"

int main()
{
    Application app;

    if (!app.initialize())
        return 1;

    return app.run();
}
