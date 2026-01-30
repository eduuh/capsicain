#include "platform/pch.h"
#include <Windows.h>  // Must come before utils.h (uses DWORD, BOOL, etc.)
#include "CommandHandler.h"

// Include dependencies from legacy code
#include "legacy/capsicain_legacy.h"
#include "platform/constants.h"
#include "legacy/traybar.h"
#include "legacy/utils.h"
#include "services/ConfigurationService.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <array>

// Selective using declarations (acceptable in .cpp files)
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::dec;

// External declarations (from capsicain.cpp global scope)
extern struct Options {
    bool debug;
    int delayForKeySequenceMS;
    bool flipZy;
    bool flipAltWinOnAppleKeyboards;
    bool LControlLWinBlocksAlphaMapping;
    bool processOnlyFirstKeyboard;
    bool holdRepeatsAllKeys;
    bool disableAHKDelay;
    std::string defaultFunction;
    bool enableMouse;
} options;

extern struct GlobalState {
    int capsicainOn;
    int activeConfig;
    std::string activeConfigName;
    int recordingMacro;
    std::vector<VKeyEvent> recordedMacros[10];
    bool deviceIsAppleKeyboard;
    int secretSequenceRecording;
} globalState;

extern struct LoopState {
    int scancode;
    bool isDownstroke;
} loopState;

extern struct InterceptionState {
    InterceptionContext interceptionContext;
} interceptionState;

extern struct AllMaps {
    std::map<uint8_t, Device> devices;
} allMaps;

extern std::array<std::string, MAX_VCODES> PRETTY_VK_LABELS;
extern std::string errorLog;

// Forward declarations of external functions
extern void reset();
extern void resetCapsNumScrollLock();
namespace capsicain { namespace services { class ConfigurationService; } }
extern void reload(capsicain::services::ConfigurationService& configService);
extern void loadAHK();
extern void unloadAHK();
extern std::vector<std::string> assembleConfig(int config);
extern void printStatus();
extern void printHelp();
extern void printKeylabels();
extern void playKeyEventSequence(std::vector<VKeyEvent> sequence);
extern void sendVKeyEvent(VKeyEvent keyEvent, bool hold);
extern std::map<uint8_t, Device>* getHardwareId(bool refresh);
extern void handleConfigSwitch(int scancode);

CommandHandler::CommandHandler(capsicain::services::ConfigurationService& configService)
    : configService_(configService)
{
    // Initialize command dispatch map
    // Note: We're not using the map yet, but setting up the infrastructure
    // For now, handle() will use switch statement to preserve exact behavior
}

bool CommandHandler::handle(int scancode)
{
    bool continueLooping = true;
    bool popupConsole = false;
    cout << endl << endl << "::";

    switch (scancode)
    {
    case SC_X:
        return handleExit();

    case SC_0:
    case SC_1:
    case SC_2:
    case SC_3:
    case SC_4:
    case SC_5:
    case SC_6:
    case SC_7:
    case SC_8:
    case SC_9:
        handleConfigSwitch(scancode);
        break;

    case SC_BACK:
        handleReset();
        break;

    case SC_T:
        handleTrayToggle();
        break;

    case SC_Q:
        handleQuit();
        break;

    case SC_W:
        handleAppleKeyboardToggle();
        break;

    case SC_E:
        handleErrorLog();
        popupConsole = true;
        break;

    case SC_R:
        handleReload();
        break;

    case SC_Y:
        handleStopAHK();
        break;

    case SC_I:
        handleShowIni();
        break;

    case SC_A:
        handleStartAHK();
        break;

    case SC_S:
        handleStatus();
        popupConsole = true;
        break;

    case SC_D:
        handleDebugToggle();
        popupConsole = configService_.getOptionsMutable().debug;
        break;

    case SC_H:
        handleHelp();
        popupConsole = true;
        break;

    case SC_J:
        handleMacroRecordStart();
        break;

    case SC_K:
        handleMacroRecordStop();
        break;

    case SC_L:
        handleMacroPlay();
        break;

    case SC_SEMI:
        handleMacroCopyToClipboard();
        break;

    case SC_Z:
        handleFlipZyToggle();
        break;

    case SC_C:
        handleShowKeyLabels();
        popupConsole = true;
        break;

    case SC_COMMA:
        handleDecreaseDelay();
        break;

    case SC_DOT:
        handleIncreaseDelay();
        break;

    case SC_M:
        handleMouseToggle();
        break;

    default:
        cout << "Unknown command";
        break;
    }

    if (popupConsole)
    {
        ShowInTaskbar();
    }

    return continueLooping;
}

// ============================================================================
// Command Handler Implementations
// ============================================================================

bool CommandHandler::handleExit()
{
    cout << endl << endl << "ESC+X :: EXIT";
    return false;
}

void CommandHandler::handleReset()
{
    cout << endl << endl << "::RESET STATE";
    reset();
    resetCapsNumScrollLock();
}

void CommandHandler::handleTrayToggle()
{
    if (IsCapsicainInTray())
    {
        cout << "Show in taskbar";
        ShowInTaskbar();
    }
    else
    {
        cout << "Show traybar";
        ShowInTraybar(globalState.activeConfig != 0, globalState.recordingMacro >= 0, globalState.activeConfig);
    }
}

void CommandHandler::handleQuit()
{
#ifdef NDEBUG
    sendVKeyEvent({ SC_ESCAPE, true });
    sendVKeyEvent({ SC_Q, true });
    sendVKeyEvent({ SC_Q, false });
    sendVKeyEvent({ SC_ESCAPE, false });
#else
    // In debug builds, this is handled by returning false from handle()
    // For now, we just passthrough the keys
    sendVKeyEvent({ SC_ESCAPE, true });
    sendVKeyEvent({ SC_Q, true });
    sendVKeyEvent({ SC_Q, false });
    sendVKeyEvent({ SC_ESCAPE, false });
#endif
}

void CommandHandler::handleAppleKeyboardToggle()
{
    configService_.getOptionsMutable().flipAltWinOnAppleKeyboards = !configService_.getOptionsMutable().flipAltWinOnAppleKeyboards;
    cout << "Flip ALT<>WIN for Apple boards: " << (configService_.getOptionsMutable().flipAltWinOnAppleKeyboards ? "ON" : "OFF") << endl;
}

void CommandHandler::handleErrorLog()
{
    cout << "ERROR LOG: " << endl << errorLog << endl;
}

void CommandHandler::handleReload()
{
    cout << "RELOAD INI";
    getHardwareId();
    reload(configService_);
    cout << endl << (globalState.deviceIsAppleKeyboard ? "APPLE keyboard (flipping Win<>Alt)" : "PC keyboard");
}

void CommandHandler::handleStopAHK()
{
    cout << "Stop AHK";
    unloadAHK();
}

void CommandHandler::handleShowIni()
{
    cout << "INI filtered for config " << globalState.activeConfigName;
    vector<string> tmpAssembledConfig = assembleConfig(globalState.activeConfig);
    for (const auto& line : tmpAssembledConfig)
        cout << endl << line;
}

void CommandHandler::handleStartAHK()
{
    cout << "Start AHK";
    loadAHK();
}

void CommandHandler::handleStatus()
{
    printStatus();
}

void CommandHandler::handleDebugToggle()
{
    configService_.getOptionsMutable().debug = !configService_.getOptionsMutable().debug;
    cout << "DEBUG mode: " << (configService_.getOptionsMutable().debug ? "ON" : "OFF");
}

void CommandHandler::handleHelp()
{
    printHelp();
}

void CommandHandler::handleMacroRecordStart()
{
    cout << "MACRO 0 START RECORDING";
    globalState.recordingMacro = 0;
    globalState.recordedMacros[0].clear();
    updateTrayIcon(true, globalState.recordingMacro >= 0, globalState.activeConfig);
}

void CommandHandler::handleMacroRecordStop()
{
    if (globalState.recordingMacro == 0)
    {
        // Remove all key-down at the end, caused by pressing ESC+K
        while (globalState.recordedMacros[0].size() > 0 && globalState.recordedMacros[0].back().isDownstroke)
            globalState.recordedMacros[0].pop_back();

        // Remove all key-up at the beginning, caused by releasing ESC+J
        while (globalState.recordedMacros[0].size() > 0 && !globalState.recordedMacros[0].front().isDownstroke)
            globalState.recordedMacros[0].erase(globalState.recordedMacros[0].begin());

        cout << "MACRO 0 STOP RECORDING (" << globalState.recordedMacros[0].size() << ")";
    }
    else
    {
        cout << "MACRO 0 RECORDING ALREADY STOPPED";
    }

    globalState.recordingMacro = -1;
    updateTrayIcon(true, globalState.recordingMacro >= 0, globalState.activeConfig);
}

void CommandHandler::handleMacroPlay()
{
    cout << "MACRO 0 PLAYBACK";
    playKeyEventSequence(globalState.recordedMacros[0]);
}

void CommandHandler::handleMacroCopyToClipboard()
{
    cout << "COPY MACRO 0 TO CLIPBOARD";
    string macro = "";
    for (const auto& key : globalState.recordedMacros[0])
    {
        if (macro.size() > 0)
            macro += "_";
        if (key.isDownstroke)
            macro += "&";
        else
            macro += "^";
        macro += PRETTY_VK_LABELS[key.vcode];
    }
    copyToClipBoard(macro);
}

void CommandHandler::handleFlipZyToggle()
{
    configService_.getOptionsMutable().flipZY = !configService_.getOptionsMutable().flipZY;
    cout << "Flip Z<>Y mode: " << (configService_.getOptionsMutable().flipZY ? "ON" : "OFF");
}

void CommandHandler::handleShowKeyLabels()
{
    cout << "List of all Key Labels for scancodes" << endl
         << "------------------------------------" << endl;
    printKeylabels();
}

void CommandHandler::handleDecreaseDelay()
{
    if (configService_.getOptionsMutable().delayForKeySequenceMS >= 1)
        configService_.getOptionsMutable().delayForKeySequenceMS -= 1;
    cout << "delay between characters in key sequences (ms): " << dec << configService_.getOptionsMutable().delayForKeySequenceMS;
}

void CommandHandler::handleIncreaseDelay()
{
    if (configService_.getOptionsMutable().delayForKeySequenceMS <= 100)
        configService_.getOptionsMutable().delayForKeySequenceMS += 1;
    cout << "delay between characters in key sequences (ms): " << dec << configService_.getOptionsMutable().delayForKeySequenceMS;
}

void CommandHandler::handleMouseToggle()
{
    configService_.getOptionsMutable().enableMouse ^= true;
    if (interceptionState.interceptionContext)
    {
        if (configService_.getOptionsMutable().enableMouse)
        {
            interception_set_filter(interceptionState.interceptionContext, interception_is_mouse,
                INTERCEPTION_FILTER_MOUSE_ALL & ~INTERCEPTION_FILTER_MOUSE_MOVE);
            cout << endl << "MOUSE INPUT ENABLED";
        }
        else
        {
            interception_set_filter(interceptionState.interceptionContext, interception_is_mouse,
                INTERCEPTION_FILTER_MOUSE_NONE);
            cout << endl << "MOUSE INPUT DISABLED";
        }
    }
}

void CommandHandler::handleBetaTest()
{
    // betaTest() function from capsicain.cpp
    // Currently commented out, leaving empty
}
