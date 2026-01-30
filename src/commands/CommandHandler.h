#pragma once

#include <functional>
#include <map>
#include <string>

/**
 * CommandHandler - Handles ESC+key command sequences
 *
 * Extracts command handling logic from the monolithic processCommand() switch.
 * Each command (ESC+X, ESC+D, ESC+R, etc.) has its own handler method.
 *
 * Uses Command Pattern with function dispatch for O(1) lookup.
 */
class CommandHandler
{
public:
    CommandHandler();

    /**
     * Handle a command keystroke (ESC was pressed, now processing the command key)
     * @param scancode The scancode of the command key (e.g., SC_X, SC_D, SC_R)
     * @return true to continue main loop, false to exit application
     */
    bool handle(int scancode);

private:
    // Command handler function type
    using CommandFunc = std::function<bool()>;

    // Dispatch map: scancode -> handler function
    std::map<int, CommandFunc> handlers_;

    // Individual command handlers
    bool handleExit();              // ESC+X: Exit application
    void handleConfigSwitch0();     // ESC+0: Switch to config 0 (disabled)
    void handleConfigSwitch1();     // ESC+1: Switch to config 1
    void handleConfigSwitch2();     // ESC+2: Switch to config 2
    void handleConfigSwitch3();     // ESC+3: Switch to config 3
    void handleConfigSwitch4();     // ESC+4: Switch to config 4
    void handleConfigSwitch5();     // ESC+5: Switch to config 5
    void handleConfigSwitch6();     // ESC+6: Switch to config 6
    void handleConfigSwitch7();     // ESC+7: Switch to config 7
    void handleConfigSwitch8();     // ESC+8: Switch to config 8
    void handleConfigSwitch9();     // ESC+9: Switch to config 9
    void handleReset();             // ESC+BACK: Reset state
    void handleTrayToggle();        // ESC+T: Toggle tray/taskbar
    void handleQuit();              // ESC+Q: Quit (debug only)
    void handleAppleKeyboardToggle(); // ESC+W: Toggle Apple keyboard mode
    void handleErrorLog();          // ESC+E: Show error log
    void handleReload();            // ESC+R: Reload configuration
    void handleStopAHK();           // ESC+Y: Stop AutoHotkey
    void handleShowIni();           // ESC+I: Show INI config
    void handleStartAHK();          // ESC+A: Start AutoHotkey
    void handleStatus();            // ESC+S: Show status
    void handleDebugToggle();       // ESC+D: Toggle debug mode
    void handleHelp();              // ESC+H: Show help
    void handleMacroRecordStart();      // ESC+J: Start macro recording
    void handleMacroRecordStop();       // ESC+K: Stop macro recording
    void handleMacroPlay();             // ESC+L: Play macro
    void handleMacroCopyToClipboard();  // ESC+;: Copy macro to clipboard
    void handleFlipZyToggle();          // ESC+Z: Toggle Z/Y flip
    void handleShowKeyLabels();         // ESC+C: Show key labels
    void handleDecreaseDelay();         // ESC+,: Decrease key sequence delay
    void handleIncreaseDelay();         // ESC+.: Increase key sequence delay
    void handleMouseToggle();           // ESC+M: Toggle mouse input
    void handleBetaTest();              // ESC+B: Beta test function
};
