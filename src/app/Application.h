#pragma once

#include <memory>
#include "commands/CommandHandler.h"
#include "ui/ConsoleUI.h"
#include "platform/interception.h"

/**
 * Application - Main application orchestrator
 *
 * Owns the main loop and coordinates all major components:
 * - CommandHandler for ESC+key commands
 * - ConsoleUI for output
 * - Interception for input
 *
 * This class extracts the main() function logic into a testable,
 * maintainable structure with proper dependency management.
 */
class Application
{
public:
    Application();
    ~Application();

    /**
     * Initialize the application
     * - Check for existing instance
     * - Create Interception context
     * - Load configuration
     * - Initialize subsystems
     *
     * @return true if initialization succeeded, false otherwise
     */
    bool initialize();

    /**
     * Run the main event loop
     * - Process keyboard/mouse events
     * - Dispatch commands
     * - Handle key transformations
     *
     * @return exit code (0 = success)
     */
    int run();

    /**
     * Shutdown and cleanup
     * - Destroy Interception context
     * - Save state if needed
     */
    void shutdown();

private:
    // Core components (owned by Application)
    CommandHandler commandHandler_;
    ConsoleUI consoleUI_;

    // Interception context
    InterceptionContext interceptionContext_;

    // Main loop state
    bool exitRequested_;

    // Helper methods
    bool initConsole();
    bool initInterception();
    bool loadConfiguration();
};
