#pragma once

#include <memory>
#include "commands/CommandHandler.h"
#include "ui/ConsoleUI.h"
#include "platform/interception.h"
#include "services/UIService.h"
#include "services/ErrorService.h"
#include "services/ProfilingService.h"
#include "services/ConfigurationService.h"

/**
 * Application - Main application orchestrator
 *
 * Owns the main loop and coordinates all major components:
 * - Services (UI, Error, Profiling, Configuration)
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

    // Service accessors (for legacy code migration)
    capsicain::services::UIService& getUIService() { return uiService_; }
    capsicain::services::ErrorService& getErrorService() { return errorService_; }
    capsicain::services::ProfilingService& getProfilingService() { return profilingService_; }
    capsicain::services::ConfigurationService& getConfigService() { return configService_; }
    ConsoleUI& getConsoleUI() { return consoleUI_; }

private:
    // UI components
    ConsoleUI consoleUI_;

    // Services (Phase 1: Utility services)
    capsicain::services::UIService uiService_;
    capsicain::services::ErrorService errorService_;
    capsicain::services::ProfilingService profilingService_;
    capsicain::services::ConfigurationService configService_;

    // Core components
    CommandHandler commandHandler_;

    // Interception context
    InterceptionContext interceptionContext_;

    // Main loop state
    bool exitRequested_;

    // Helper methods
    bool initConsole();
    bool initInterception();
    bool loadConfiguration();
};
