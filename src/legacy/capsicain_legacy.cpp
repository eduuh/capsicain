#pragma once

#include "platform/pch.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <set>
#include <map>
#include <array>
#include <algorithm>
#include <string>
#include <sstream>
#include <chrono>
#include <Windows.h>  //for Sleep()

#include "legacy/capsicain_legacy.h"
#include "platform/constants.h"
#include "legacy/modifiers.h"
#include "legacy/scancodes.h"
#include "platform/resource.h"
#include "legacy/led.h"
#include "legacy/utils.h"
#include "commands/CommandHandler.h"
#include "ui/ConsoleUI.h"
#include "app/Application.h"
#include "domain/TapDetector.h"
#include "domain/ModifierTracker.h"
#include "domain/KeyMapper.h"
#include "domain/ComboMatcher.h"

// AHK typedefs and struct moved to AutoHotkeyService (Phase 4)

// Selective using declarations
using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;
using std::map;
using std::set;
using std::hex;
using std::dec;
using std::setw;
using std::to_string;
using std::uppercase;
namespace chrono = std::chrono;

// Global pointers for legacy compatibility (Phase 1-5 migration)
static capsicain::services::UIService* g_uiService = nullptr;
static capsicain::services::ErrorService* g_errorService = nullptr;
static capsicain::services::ProfilingService* g_profilingService = nullptr;
static capsicain::services::AutoHotkeyService* g_ahkService = nullptr;
static capsicain::services::RuntimeStateService* g_runtimeState = nullptr;

// Adapter to bridge legacy global functions to IModifierQuery interface
class LegacyModifierQuery : public capsicain::domain::IModifierQuery {
public:
    bool isModifier(capsicain::domain::VKeyCode vcode) const override {
        return ::isModifier(static_cast<int>(vcode));
    }

    MOD getModifierBitmask(capsicain::domain::VKeyCode vcode) const override {
        return ::getModifierBitmaskForVcode(static_cast<int>(vcode));
    }
};

//try out if we can play doom when we have a TMK style temp layer shift key
/*
int TESTING_LAYER_SHIFT_KEY = SC_APPS;
int TESTING_LAYER_SHIFT_TO = 9;     // tmp shift to this layer
int TESTING_LAYER_SHIFT_FROM = -1;  // original layer. <0 means undefined
*/

std::array<string, MAX_VCODES> PRETTY_VK_LABELS; // contains e.g. [SC_ESCAPE]="ESC"; all VKs incl. > 0xFF

vector<string> sanitizedIniContent;  //loaded on startup and reset

//only written on ini load
struct Globals
{
    string iniVersion = "unnamed version - add 'iniVersion xyz123' to capsicain.ini";
    int activeConfigOnStartup = DEFAULT_ACTIVE_CONFIG;
    bool startMinimized = false;
    bool startInTraybar = false;
    bool startAHK = false;
    int capsicainOnOffKey = -1;
    bool protectConsole = true; //drop Pause and Break signals when console is foreground
    bool translateMessyKeys = true; //translate various DOS keys (e.g. Ctrl+Pause=SC_Break -> SC_Pause, Alt+Print=SC_altprint -> sc_print)
    set<int> disableEscKey;
    set<int> forwardEscKey;
} globals;
static const struct Globals defaultGlobals;

//can be toggled with ESC commands
struct Options
{
    bool debug = false;
    int delayForKeySequenceMS = DEFAULT_DELAY_FOR_KEY_SEQUENCE_MS;
    bool flipZy = false;
    bool flipAltWinOnAppleKeyboards = false;
    bool LControlLWinBlocksAlphaMapping = false;
    bool processOnlyFirstKeyboard = false;
    bool holdRepeatsAllKeys = false;
    bool disableAHKDelay = false;
    string defaultFunction = "key(%s, m)";
    bool enableMouse = false;
} options;
static const struct Options defaultOptions;

struct ModifierCombo
{
    int vkey = SC_NOP;
    unsigned char deadkey = 0;
    MOD modAnd = 0;
    MOD modOr = 0;
    MOD modNot = 0;
    MOD modTap = 0;
    MOD modTapAnd = 0;
    DEV devAnd = 0;
    DEV devNot = 0;
    vector<VKeyEvent> keyEventSequence;
};

struct AllMaps
{
    //inkey outkey (tapped)
    //-1 = undefined key
    std::array<std::array<int, REWIRE_COLS>, REWIRE_ROWS> rewiremap{}; //MUST initialize this manually to -1 !!

    map<string, vector<ModifierCombo> > modCombos{
        { INI_TAG_COMBOS, {} },
        { INI_TAG_UPCOMBOS, {} },
        { INI_TAG_TAPCOMBOS, {} },
        { INI_TAG_SLOWCOMBOS, {} },
        { INI_TAG_REPEATCOMBOS, {} }
    };

    // Cached converted combos for performance (avoid conversion on every keystroke)
    map<string, vector<capsicain::domain::ComboRule> > convertedCombos{
        { INI_TAG_COMBOS, {} },
        { INI_TAG_UPCOMBOS, {} },
        { INI_TAG_TAPCOMBOS, {} },
        { INI_TAG_SLOWCOMBOS, {} },
        { INI_TAG_REPEATCOMBOS, {} }
    };

    std::array<int, MAX_VCODES> alphamap{}; //MUST initialize this manually to 1 1, 2 2, 3 3, ...

    map<int, Executable> executables;
    map<uint8_t, Device> devices;
} allMaps;

struct InterceptionState
{
    int newKeyboardCounter = 0;
    InterceptionContext interceptionContext = nullptr;
    InterceptionDevice interceptionDevice = 0;
    InterceptionDevice previousInterceptionDevice = 0;
    InterceptionKeyStroke currentIKstroke = { SC_NOP, 0 };
    InterceptionKeyStroke previousIKstroke1 = { SC_NOP, 0 }; //remember history
    InterceptionKeyStroke previousIKstroke2 = { SC_NOP, 0 };
    InterceptionDevice lastMouse = 0;
    InterceptionDevice lastKeyboard = 0;
} interceptionState;

struct GlobalState
{
    bool capsicainOn = true;

    int  activeConfig = 0;
    string activeConfigName = DEFAULT_ACTIVE_CONFIG_NAME;
    int previousConfig = 1; // switch to this on func(CONFIGPREVIOUS)

    bool realEscapeIsDown = false;

    string deviceIdKeyboard = "";
    string includeDeviceId = "";
    string excludeDeviceId = "";

    bool deviceIsAppleKeyboard = false;

    int keysDownSentCounter = 0;  //tracks how many keys are actually down that Windows knows about
    std::array<bool, 256> keysDownSent{};  //Remember all forwarded to Windows. Sent keys must be 8 bit
    std::array<bool, 256> keysDownTempReleased{};  //Remember all keys that were temporarily released, e.g. to send an Alt-Numpad combo
    std::array<set<int>, VK_MAX> holdKeys;  //Remember all replaced hold() keys while the physical key is still down

    bool secretSequenceRecording = false;
    bool secretSequencePlayback = false;
    int recordingMacro = -1; //-1: not recording. 1..MAX_SIMPLE_MACROS : this is currently recording. 0=currently recording the 'hard' ESC+J macro
    std::array<vector<VKeyEvent>, MAX_NUM_MACROS> recordedMacros;  // [0] stores the 'hard' macro
} globalState;
static const struct GlobalState defaultGlobalState;

struct ModifierState
{
    unsigned char activeDeadkey = 0;  //it's not really a modifier though...
    MOD modifierDown = 0;
    MOD modifierTapped = 0;
    MOD modifierForceDown = 0;
    vector<VKeyEvent> modsTempAltered;
    int tapAndHoldKey = -1; //remember the tap-and-hold key as long as it is down
} modifierState;
static const struct ModifierState defaultModifierState;

struct LoopState
{
    unsigned char scancode = SC_NOP; //hardware code sent by Interception
    int vcode = -1; //key code used internally; equals scancode or a Virtual code > FF
    bool isDownstroke = false;
    bool isModifier = false;
    bool tapped = false;
    bool tappedSlow = false;  //autorepeat set in before key release
    bool tapHoldMake = false;  //tap-and-hold action (like LAlt > mod12 // LAlt)
    bool repeat = false;

    vector<VKeyEvent> resultingVKeyEventSequence;

} loopState;
static const struct LoopState defaultLoopState;

struct ProfilingTimer
{
    chrono::steady_clock::time_point timepointStopwatch;
    chrono::steady_clock::time_point timepointPreviousKeyEvent;
    chrono::steady_clock::time_point timepointLoopStart = std::chrono::steady_clock::now();

    int countIncoming = 0;
    int countOutgoing = 0;
    unsigned long totalMappingTimeUS = 0;
    unsigned long totalSendingTimeUS = 0;
    unsigned long worstMappingTimeUS = 0;
    unsigned long worstSendingTimeUS = 0;

    chrono::steady_clock::time_point getTimepointNow()
    {
        return std::chrono::steady_clock::now();
    }
    unsigned long stopwatchRestart()
    {
        unsigned long dura = stopwatchReadUS();
        timepointStopwatch = std::chrono::steady_clock::now();
        return dura;
    }
    unsigned long stopwatchReadUS()
    {
        return (unsigned long)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timepointStopwatch).count();
    }
} profiler;
static const struct ProfilingTimer defaultProfiler;

string errorLog = "";  // Legacy global - will be removed in later phase
void error(const string& txt)
{
    // Use ErrorService if available, otherwise fall back to legacy behavior
    if (g_errorService) {
        g_errorService->logError(txt);
        errorLog = g_errorService->getErrorLog();  // Keep legacy global in sync
    } else {
        cout << endl << "ERROR: " << txt << endl;
        errorLog += "\r\n" + txt;
    }
}

string getTimestamp()
{
    auto start = std::chrono::system_clock::now();
    auto legacyStart = std::chrono::system_clock::to_time_t(start);
    char tmBuff[30];
    ctime_s(tmBuff, sizeof(tmBuff), &legacyStart);
    return tmBuff;
}


string getPrettyVKLabelPadded(int vcode, int resultLength)
{
    string label = PRETTY_VK_LABELS[vcode];
    if (resultLength > label.size())
        label.insert(0, resultLength - label.size(), ' ');
    return label;
}
string getPrettyVKLabel(int vcode)
{
    return PRETTY_VK_LABELS[vcode];
}

void InterceptionSendCurrentKeystroke()
{
    interception_send(interceptionState.interceptionContext, interceptionState.interceptionDevice, (InterceptionStroke*)&interceptionState.currentIKstroke, 1);
}

void loadAHK()
{
    if (g_ahkService) {
        if (!g_ahkService->initializeDLL()) {
            return;
        }
        g_ahkService->loadScriptFromIni();
    }
}

void unloadAHK()
{
    if (g_ahkService) {
        g_ahkService->shutdown();
    }
}

int mousetoKey(InterceptionMouseStroke &mstroke, InterceptionKeyStroke *kstroke)
{
    auto state = mstroke.state;
    auto roll = mstroke.rolling;
    int n = 0;
    if (state & INTERCEPTION_MOUSE_BUTTON_1_DOWN || state & INTERCEPTION_MOUSE_BUTTON_1_UP)
    {
        kstroke[n].code = VM_LEFT;
        kstroke[n].state = (int)(bool)!(state & INTERCEPTION_MOUSE_BUTTON_1_DOWN);
        n++;
    }
    if (state & INTERCEPTION_MOUSE_BUTTON_2_DOWN || state & INTERCEPTION_MOUSE_BUTTON_2_UP)
    {
        kstroke[n].code = VM_RIGHT;
        kstroke[n].state = (int)(bool)!(state & INTERCEPTION_MOUSE_BUTTON_2_DOWN);
        n++;
    }
    if (state & INTERCEPTION_MOUSE_BUTTON_3_DOWN || state & INTERCEPTION_MOUSE_BUTTON_3_UP)
    {
        kstroke[n].code = VM_MIDDLE;
        kstroke[n].state = (int)(bool)!(state & INTERCEPTION_MOUSE_BUTTON_3_DOWN);
        n++;
    }
    if (state & INTERCEPTION_MOUSE_BUTTON_4_DOWN || state & INTERCEPTION_MOUSE_BUTTON_4_UP)
    {
        kstroke[n].code = VM_BUTTON4;
        kstroke[n].state = (int)(bool)!(state & INTERCEPTION_MOUSE_BUTTON_4_DOWN);
        n++;
    }
    if (state & INTERCEPTION_MOUSE_BUTTON_5_DOWN || state & INTERCEPTION_MOUSE_BUTTON_5_UP)
    {
        kstroke[n].code = VM_BUTTON5;
        kstroke[n].state = (int)(bool)!(state & INTERCEPTION_MOUSE_BUTTON_5_DOWN);
        n++;
    }
    if(state & INTERCEPTION_MOUSE_WHEEL)
    {
        if (roll > 0)
            kstroke[n].code = VM_WHEEL_UP;
        else if (roll < 0)
            kstroke[n].code = VM_WHEEL_DOWN;
        kstroke[n].state = 0;
        n++;
    }
    if(state & INTERCEPTION_MOUSE_HWHEEL)
    {
        if (roll < 0)
            kstroke[n].code = VM_WHEEL_LEFT;
        else if (roll > 0)
            kstroke[n].code = VM_WHEEL_RIGHT;
        kstroke[n].state = 0;
        n++;
    }
    return n;
}

// Forward declare Application class
class Application;

// Main implementation - extracted from main() to allow Application wrapper
int capsicain_main_impl(Application* app)
{
    if (!initConsoleWindow())
    {
        std::cout << endl << "Capsicain already running - exiting..." << endl;
        Sleep(5000);
        return 0;
    }

    // Get services from Application (services already initialized in Application::run())
    auto& uiService = app->getUIService();
    auto& errorService = app->getErrorService();
    auto& profilingService = app->getProfilingService();
    auto& configService = app->getConfigService();
    ConsoleUI& consoleUI = app->getConsoleUI();

    // Set legacy globals for backwards compatibility
    g_uiService = &uiService;
    g_errorService = &errorService;
    g_profilingService = &profilingService;
    g_ahkService = &app->getAHKService();
    g_runtimeState = &app->getRuntimeState();

    interceptionState.interceptionContext = interception_create_context();

    if constexpr (ENABLE_PROFILING) profilingService.stopwatchRestart();

    consoleUI.printHeader();

    // UIService and ConfigService already initialized in Application::run()
    // Just verify INI was loaded
    if (!configService.isLoaded())
    {
        std::cout << endl << "No capsicain.ini - exiting..." << endl;
        Sleep(5000);
        return 0;
    }

    // Populate legacy globals from services (temporary during Phase 1 migration)
    // Phase 1: Keep globals but populate from services
    sanitizedIniContent = configService.getIniContent();
    // Copy labels from service to global array
    for (int i = 0; i < MAX_VCODES; ++i) {
        PRETTY_VK_LABELS[i] = uiService.getLabel(i);
    }

    // Phase 2: Parse INI into configService, then copy to legacy globals
    parseIniGlobals(configService);

    // Copy GlobalSettings to legacy globals struct
    const auto& settings = configService.getGlobalSettings();
    globals.iniVersion = settings.iniVersion;
    globals.activeConfigOnStartup = settings.activeConfigOnStartup;
    globals.startMinimized = settings.startMinimized;
    globals.startInTraybar = settings.startInTraybar;
    globals.startAHK = settings.startAHK;
    globals.capsicainOnOffKey = settings.capsicainOnOffKey;
    globals.protectConsole = settings.protectConsole;
    globals.translateMessyKeys = settings.translateMessyKeys;
    globals.disableEscKey = settings.disableEscKey;
    globals.forwardEscKey = settings.forwardEscKey;

    // Copy RuntimeOptions to legacy options struct
    const auto& opts = configService.getOptions();
    options.debug = opts.debug;
    options.delayForKeySequenceMS = opts.delayForKeySequenceMS;
    options.flipZy = opts.flipZY;  // Note: different spelling
    options.flipAltWinOnAppleKeyboards = opts.flipAltWinOnAppleKeyboards;
    options.LControlLWinBlocksAlphaMapping = opts.lControlLWinBlocksAlphaMapping;  // Note: different casing
    options.processOnlyFirstKeyboard = opts.processOnlyFirstKeyboard;
    options.holdRepeatsAllKeys = opts.holdRepeatsAllKeys;
    options.disableAHKDelay = opts.disableAHKDelay;
    options.defaultFunction = opts.defaultFunction;
    options.enableMouse = opts.enableMouse;

    if (globals.startAHK)
        loadAHK();

    switchConfig(globals.activeConfigOnStartup, true);

    cout << endl << endl << "[ESC] + [X] to stop." << endl << "[ESC] + [H] for Help";
    cout << endl << endl << "capsicain running.... ";

    if (globals.startInTraybar)
        ShowInTraybar(globalState.activeConfig != 0, globalState.recordingMacro >= 0, globalState.activeConfig);
    else if (globals.startMinimized)
        ShowInTaskbarMinimized();

    if (globals.capsicainOnOffKey == SC_NUMLOCK
        || globals.capsicainOnOffKey == SC_SCRLOCK
        || globals.capsicainOnOffKey == SC_CAPS)
    {
        setLED(globals.capsicainOnOffKey, true);
    }

    if constexpr (ENABLE_PROFILING) cout << endl << endl << "Profiling enabled in this build" << endl << "Startup time: " << profiler.stopwatchReadUS() / 1000 << " ms" << endl;

    raise_process_priority(); //careful: if we spam key events, other processes get no timeslots to process them. Sleep a bit...

    interception_set_filter(interceptionState.interceptionContext, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);
    if (options.enableMouse)
        interception_set_filter(interceptionState.interceptionContext, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_ALL & ~INTERCEPTION_FILTER_MOUSE_MOVE);
    else
        interception_set_filter(interceptionState.interceptionContext, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_NONE);

    InterceptionDevice device;
    InterceptionStroke stroke;

    // Initialize command handler for ESC+key sequences (Phase 4: inject configService and uiService)
    CommandHandler commandHandler(configService, uiService);

    // Initialize domain components for refactored key processing
    capsicain::TapDetector tapDetector;
    capsicain::ModifierTracker modifierTracker;
    capsicain::domain::KeyMapper keyMapper;
    capsicain::domain::ComboMatcher comboMatcher;
    LegacyModifierQuery modifierQuery;

    //CORE LOOP
    bool exit = false;
    while (!exit)
    {
        //wait for the next key from Interception
        int n = interception_receive(interceptionState.interceptionContext,
            device = interception_wait_with_timeout(interceptionState.interceptionContext, 2),
            &stroke, 1);

        InterceptionKeyStroke strokes[10] = {0};

        if (device && n)
        {
            if (interception_is_mouse(device))
            {
                InterceptionMouseStroke &mstroke = *(InterceptionMouseStroke *) &stroke;
                interceptionState.lastMouse = device;
                int m = mousetoKey(mstroke, (InterceptionKeyStroke *)&strokes);
                if (m == 0)
                    continue;
                n = m;
            }
            else
            {
                interceptionState.lastKeyboard = device;
                strokes[0] = *(InterceptionKeyStroke *)&stroke;
            }

            if (allMaps.devices.find(device) == allMaps.devices.end())
                getHardwareId();

            globalState.deviceIdKeyboard = allMaps.devices[device].id;
            globalState.deviceIsAppleKeyboard = allMaps.devices[device].apple;

            for (int i = 0; i < n; ++i)
            {
                //remember previous two keys to detect tapping and Pause sequence
                interceptionState.previousIKstroke2 = interceptionState.previousIKstroke1;
                interceptionState.previousIKstroke1 = interceptionState.currentIKstroke;

                interceptionState.interceptionDevice = device;
                interceptionState.currentIKstroke = strokes[i];

                if constexpr (ENABLE_PROFILING)
                {
                    //Measure Timing. sleep() is not precise; just a rough outline. Expect occasional 30ms sleeps from thread scheduling.
                    profiler.timepointPreviousKeyEvent = profiler.timepointLoopStart;
                    profiler.timepointLoopStart = profiler.getTimepointNow();
                    profiler.stopwatchRestart();
                    profiler.countIncoming++;
                }

                //low level debugging, show incoming raw key
                if constexpr (ENABLE_TRACE) consoleUI.printIKStrokeState(interceptionState.currentIKstroke);

                //clear loop state
                loopState = defaultLoopState;

                //copy InterceptionKeyStroke (unpleasant to use) to plain VKeyEvent
                VKeyEvent originalVKeyEvent = convertIkstroke2VKeyEvent(interceptionState.currentIKstroke);
                loopState.scancode = originalVKeyEvent.vcode;  //scancode is write-once (except for the AppleWinAlt option)
                loopState.vcode = loopState.scancode;          //vcode may be altered below
                loopState.isDownstroke = originalVKeyEvent.isDownstroke;

                //if GLOBAL capsicainEnableDisable is configured, it toggles the ON/OFF state
                if (globals.capsicainOnOffKey != -1)
                {
                    if (processOnOffKey())
                        continue;
                }
                //if disabled, just forward
                if (!globalState.capsicainOn)
                {
                    InterceptionSendCurrentKeystroke();
                    continue;
                }

                IFDEBUG if(globalState.activeConfig == 0) cout << ". ";

                //ignore secondary keyboard?
                if (options.processOnlyFirstKeyboard
                    && (interceptionState.previousInterceptionDevice != 0)
                    && (interceptionState.previousInterceptionDevice != interceptionState.interceptionDevice))
                {
                    IFDEBUG cout << endl << "Ignore 2nd board (" << interceptionState.interceptionDevice << ") scancode: " << interceptionState.currentIKstroke.code;
                    InterceptionSendCurrentKeystroke();
                    continue;
                }

                //device id changed / check for Apple Keyboard
                if (interceptionState.previousInterceptionDevice == 0    //startup
                    || interceptionState.previousInterceptionDevice != interceptionState.interceptionDevice)  //keyboard changed
                {
                    //getHardwareId();
                    //detail to debug the "new device after sleep, reboot after 10 new devices"
                    if constexpr (ENABLE_TRACE) cout << endl
                        << "<" << endl
                        << "new keyboard: " << (globalState.deviceIsAppleKeyboard ? "Apple keyboard" : "IBM keyboard") << endl
                        << "new keyboard count: " << ++interceptionState.newKeyboardCounter << endl
                        << "keyboard device id: " << globalState.deviceIdKeyboard << endl
                        << "interceptionDevice: " << interceptionState.interceptionDevice << endl
                        << getTimestamp()
                        << ">" << endl;



                    interceptionState.previousInterceptionDevice = interceptionState.interceptionDevice;
                }

                //sanity check
                if (interceptionState.currentIKstroke.code >= 0x80 && interceptionState.currentIKstroke.code < VM_LEFT)
                {
                    error("Received unexpected extended Interception Key Stroke code > 0x79: " + to_string(interceptionState.currentIKstroke.code));
                    cout << endl << "Please open a ticket on github";
                    continue;
                }
                if (interceptionState.currentIKstroke.code == 0)
                {
                    error("Received unexpected SC_NOP Key Stroke code 0. Ignoring this.");
                    continue;
                }

                //ESC Commands
                if (loopState.scancode == SC_ESCAPE)
                {
                    IFDEBUG cout << endl << "(Hard ESC" << (loopState.isDownstroke ? "v " : "^ ") << ")";
                    globalState.realEscapeIsDown = loopState.isDownstroke;

                    //stop macro recording?
                    if (globalState.recordingMacro > 0)
                    {
                        IFDEBUG cout << endl << "Stop recording macro #" << globalState.recordingMacro;
                        //wrap macro in tokens to tmprelease / restore keys, to deal with the physical 'Ctrl down' that started the macro
                        if (globalState.recordedMacros[globalState.recordingMacro].size() > 0)
                            globalState.secretSequenceRecording = false;
                        {
                            globalState.recordedMacros[globalState.recordingMacro].push_back({ VK_CPS_TEMPRESTOREKEYS,true });
                            globalState.recordedMacros[globalState.recordingMacro].insert(globalState.recordedMacros[globalState.recordingMacro].begin(), { VK_CPS_TEMPRELEASEKEYS,true });
                        }
                        globalState.recordingMacro = -1;
                        updateTrayIcon(true, globalState.recordingMacro >= 0, globalState.activeConfig);
                        continue;
                    }
                }
                else if (globalState.realEscapeIsDown && loopState.isDownstroke)
                {
                    if (globals.forwardEscKey.find(loopState.scancode) == globals.forwardEscKey.end())
                        continue;
                }
                else if (globalState.realEscapeIsDown && !loopState.isDownstroke)
                {
                    if (globals.disableEscKey.find(loopState.scancode) == globals.disableEscKey.end())
                    {
                        if (commandHandler.handle(loopState.scancode))
                        {
                            continue;
                        }
                        else
                        {
                            setLED(SC_NOP, true); // sync LEDs with Windows state.
                            ShowInTaskbar(); //exit
                            exit = true;
                        }
                    }
                    if (globals.forwardEscKey.find(loopState.scancode) == globals.forwardEscKey.end())
                        continue;
                }

                //TESTING the layer shift feature
                /*
                if (loopState.vcode == TESTING_LAYER_SHIFT_KEY)
                {
                    if (loopState.isDownstroke)
                    {
                        if (globalState.activeConfig != TESTING_LAYER_SHIFT_TO)
                        {
                            TESTING_LAYER_SHIFT_FROM = globalState.activeConfig;
                            switchConfig(TESTING_LAYER_SHIFT_TO, false);
                        }
                    }
                    else if (TESTING_LAYER_SHIFT_FROM >= 0)
                    {
                        if (TESTING_LAYER_SHIFT_FROM != globalState.activeConfig)
                        {
                            switchConfig(TESTING_LAYER_SHIFT_FROM, false);
                        }

                        TESTING_LAYER_SHIFT_FROM = -1;
                    }

                    continue;
                }
                */
                
                //Config 0: standard keyboard, no further processing, just forward everything
                if (globalState.activeConfig == DISABLED_CONFIG_NUMBER)
                {
                    InterceptionSendCurrentKeystroke();
                    continue;
                }

                //consider include/exclude deviceID options
                if (!globalState.includeDeviceId.empty()
                    && globalState.deviceIdKeyboard.find(globalState.includeDeviceId) == string::npos)
                {
                    IFDEBUG cout << endl << "Ignore board, deviceId is not included with this config";
                    InterceptionSendCurrentKeystroke();
                    continue;
                }
                if (!globalState.excludeDeviceId.empty()
                    && globalState.deviceIdKeyboard.find(globalState.excludeDeviceId) != string::npos)
                {
                    IFDEBUG cout << endl << "Ignore board, deviceId is excluded in this config";
                    InterceptionSendCurrentKeystroke();
                    continue;
                }



                //flip Win+Alt only for Apple keyboards.
                if (options.flipAltWinOnAppleKeyboards && globalState.deviceIsAppleKeyboard)
                {
                    switch (loopState.vcode)
                    {
                    case SC_LALT: loopState.vcode = SC_LWIN; break;
                    case SC_LWIN: loopState.vcode = SC_LALT; break;
                    case SC_RALT: loopState.vcode = SC_RWIN; break;
                    case SC_RWIN: loopState.vcode = SC_RALT; break;
                    }

                    loopState.scancode = loopState.vcode;       //only time where scancode is rewritten. Simplifies tapping and rewiring
                }

                //Handle Sysrq, ScrLock, Pause, NumLock
                if (!processMessyKeys())
                    continue;

                //Tapdance - use refactored TapDetector
                {
                    // Convert InterceptionKeyStrokes to KeyEvents for TapDetector
                    capsicain::KeyEvent currentEvent{
                        static_cast<int>(interceptionState.currentIKstroke.code),
                        (interceptionState.currentIKstroke.state & 1) == 0  // isDownstroke (state & 1 == 0 means down)
                    };
                    capsicain::KeyEvent prev1Event{
                        static_cast<int>(interceptionState.previousIKstroke1.code),
                        (interceptionState.previousIKstroke1.state & 1) == 0
                    };
                    capsicain::KeyEvent prev2Event{
                        static_cast<int>(interceptionState.previousIKstroke2.code),
                        (interceptionState.previousIKstroke2.state & 1) == 0
                    };

                    // Detect tap patterns using refactored domain component
                    capsicain::TapResult tapResult = tapDetector.detect(
                        currentEvent, prev1Event, prev2Event,
                        interceptionState.currentIKstroke.state,
                        interceptionState.previousIKstroke1.state,
                        interceptionState.previousIKstroke2.state
                    );

                    // Store results back into loopState (for now, to maintain compatibility)
                    loopState.tapped = tapResult.tapped;
                    loopState.tappedSlow = tapResult.tappedSlow;
                    loopState.tapHoldMake = tapResult.tapHoldMake;
                    loopState.repeat = tapResult.repeat;
                }
                //slow tap breaks tapping
                if (loopState.tappedSlow)
                {
                    modifierState.modifierTapped = 0;
                    modifierTracker.clearAllTapped();  // Sync to ModifierTracker
                }

                //hard rewire all REWIREd keys - use refactored KeyMapper
                {
                    // Build RewireEntry from legacy rewiremap
                    capsicain::domain::RewireEntry rewireEntry;
                    rewireEntry.outKey = allMaps.rewiremap[loopState.vcode][REWIRE_OUT];
                    rewireEntry.tapKey = allMaps.rewiremap[loopState.scancode][REWIRE_TAP];
                    rewireEntry.tapHoldKey = allMaps.rewiremap[loopState.scancode][REWIRE_TAPHOLD];

                    // Build RewireContext
                    capsicain::domain::RewireContext rewireContext;
                    rewireContext.scancode = loopState.scancode;
                    rewireContext.vcode = loopState.vcode;
                    rewireContext.isDownstroke = loopState.isDownstroke;
                    rewireContext.isTapped = loopState.tapped;
                    rewireContext.isTapHoldMake = loopState.tapHoldMake;
                    rewireContext.activeTapHoldKey = modifierState.tapAndHoldKey;

                    // Apply rewire mapping using refactored domain component
                    capsicain::domain::RewireResult rewireResult = keyMapper.mapRewire(
                        rewireContext, rewireEntry, &modifierQuery
                    );

                    // Apply results back to loopState and modifierState
                    loopState.vcode = rewireResult.outputKey;
                    loopState.isModifier = rewireResult.isModifier;

                    // Add any generated events to the sequence
                    for (const auto& evt : rewireResult.eventSequence) {
                        loopState.resultingVKeyEventSequence.push_back({
                            static_cast<int>(evt.keyCode),
                            evt.isDown
                        });
                    }

                    // Update modifier state based on rewire result
                    if (rewireResult.modifiersToClear != 0) {
                        modifierState.modifierDown &= ~rewireResult.modifiersToClear;
                    }
                    if (rewireResult.tappedToClear != 0) {
                        modifierState.modifierTapped &= ~rewireResult.tappedToClear;
                        modifierTracker.clearAllTapped();  // Sync: clear all tapped in tracker too
                    }

                    // Update tap-hold key
                    if (rewireResult.newTapHoldKey != 0) {
                        if (rewireResult.newTapHoldKey == -1) {
                            modifierState.tapAndHoldKey = -1;
                        } else {
                            modifierState.tapAndHoldKey = rewireResult.newTapHoldKey;
                        }
                        modifierTracker.setTapHoldKey(modifierState.tapAndHoldKey);  // Sync to tracker
                    }

                    // Check if key should be suppressed
                    if (rewireResult.shouldNop) {
                        continue;
                    }
                }
                if (loopState.vcode == SC_NOP)   //rewired to NOP to disable keys
                {
                    IFDEBUG cout << " (r2NOP)";
                    continue;
                }

                IFDEBUG
                {
                    cout << endl;
                    if constexpr (ENABLE_PROFILING) cout << "(" << setw(5) << dec << timeBetweenTimepointsUS(profiler.timepointPreviousKeyEvent, profiler.timepointLoopStart) / 1000 << " m) ";
                    consoleUI.printLoopState1Input();
                }

                //evaluate modifiers - use refactored ModifierTracker
                {
                    // Use ModifierTracker to update modifier state
                    modifierTracker.update(loopState.vcode, loopState.isDownstroke, loopState.tapped);

                    // Sync ModifierTracker state back to global modifierState (for compatibility)
                    modifierState.modifierDown = modifierTracker.getDownMask();
                    modifierState.modifierTapped = modifierTracker.getTappedMask();
                    modifierState.modifierForceDown = modifierTracker.getForcedMask();
                    modifierState.activeDeadkey = modifierTracker.getDeadkey();
                    modifierState.tapAndHoldKey = modifierTracker.getTapHoldKey();
                }

                IFDEBUG consoleUI.printLoopState2Modifier();

                //evaluate modified keys - use refactored ComboMatcher
                {
                    // Build ComboMatchContext
                    capsicain::domain::ComboMatchContext comboContext;
                    comboContext.currentKey = loopState.vcode;
                    comboContext.modifiersDown = modifierState.modifierDown;
                    comboContext.modifiersTapped = modifierState.modifierTapped;
                    // Convert device ID to bitmask (device IDs are 1-20, bitmask is 1 << (id-1))
                    comboContext.deviceMask = (interceptionState.interceptionDevice > 0)
                        ? (1 << (interceptionState.interceptionDevice - 1))
                        : 0;
                    comboContext.activeDeadkey = modifierState.activeDeadkey;

                    // Match combos using refactored domain component with cached converted combos
                    capsicain::domain::ComboMatchResult comboResult;

                    if (loopState.isDownstroke) {
                        comboResult = comboMatcher.matchDownstroke(
                            allMaps.convertedCombos[INI_TAG_COMBOS],
                            allMaps.convertedCombos[INI_TAG_REPEATCOMBOS],
                            comboContext,
                            loopState.repeat
                        );
                    } else {
                        comboResult = comboMatcher.matchUpstroke(
                            allMaps.convertedCombos[INI_TAG_UPCOMBOS],
                            allMaps.convertedCombos[INI_TAG_TAPCOMBOS],
                            allMaps.convertedCombos[INI_TAG_SLOWCOMBOS],
                            comboContext,
                            loopState.tapped,
                            loopState.tappedSlow
                        );
                    }

                    // Apply combo result
                    if (comboResult.matched) {
                        // Convert ComboKeyEvent sequence back to VKeyEvent
                        loopState.resultingVKeyEventSequence.clear();
                        for (const auto& evt : comboResult.resultSequence) {
                            loopState.resultingVKeyEventSequence.push_back({
                                static_cast<int>(evt.keyCode),
                                evt.isDown
                            });
                        }

                        // Clear tapped state if needed
                        if (comboResult.shouldClearTapped) {
                            modifierState.modifierTapped = 0;
                            modifierTracker.clearAllTapped();
                        }
                    }

                    // Clear deadkey for non-modifier keys
                    if (!loopState.isModifier) {
                        modifierState.activeDeadkey = 0;
                    }
                }

                //alphakeys: basic character key layout - use refactored KeyMapper
                {
                    // Build AlphaMapOptions
                    capsicain::domain::AlphaMapOptions alphaOptions;
                    alphaOptions.flipZY = options.flipZy;
                    alphaOptions.ctrlWinBlocksAlphaMapping = options.LControlLWinBlocksAlphaMapping;

                    // Apply alpha mapping using refactored domain component
                    capsicain::domain::AlphaMapResult alphaResult = keyMapper.mapAlpha(
                        loopState.vcode,
                        allMaps.alphamap.data(),
                        alphaOptions,
                        loopState.isModifier,
                        IS_LCTRL_DOWN,
                        IS_LWIN_DOWN
                    );

                    // Update vcode with mapped result
                    loopState.vcode = alphaResult.mappedKey;
                }

                //break tapped state?
                if (!isModifier(loopState.vcode))
                {
                    modifierState.modifierTapped = 0;
                    modifierTracker.clearAllTapped();  // Sync to ModifierTracker
                }

                if constexpr (ENABLE_PROFILING)
                {
                unsigned long mappingtime = profiler.stopwatchRestart();
                profiler.totalMappingTimeUS += mappingtime;
                profiler.countOutgoing++;
                if (mappingtime > profiler.worstMappingTimeUS)
                    profiler.worstMappingTimeUS = mappingtime;
                IFDEBUG consoleUI.printLoopStateMappingTime(mappingtime);
                }

                sendResultingKeyOrSequence();
                if constexpr (ENABLE_PROFILING)
                {
                unsigned long sendingtime = profiler.stopwatchReadUS();
                profiler.totalSendingTimeUS += sendingtime;
                if (sendingtime > profiler.worstSendingTimeUS)
                    profiler.worstSendingTimeUS = sendingtime;
                if (sendingtime > 1000)
                    cout << "\t (slow send: " << dec << sendingtime << " u)";
                }

                IFDEBUG consoleUI.printLoopState4TapState();
            }
        }
        else
        {
            // TODO: do other stuff in the loop
        }
    }
    interception_destroy_context(interceptionState.interceptionContext);

    cout << endl << "bye" << endl;
    return 0;
}
////////////////////////////////////END MAIN IMPL//////////////////////////////////////

// New main() - uses Application class for better organization
// main() has been moved to root main.cpp for cleaner project structure

////////////////////////////////////END MAIN//////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

void betaTest() //ESC+B
{
    setLED(SC_CAPS, true);

    //test SendInput
    //{
    //    // Create a keyboard event structure
    //    INPUT ip;
    //    ip.type = INPUT_KEYBOARD;
    //    ip.ki.time = 0;
    //    ip.ki.dwExtraInfo = 0;

    //    // Press a unicode "key"
    //    ip.ki.dwFlags = KEYEVENTF_UNICODE;
    //    ip.ki.wVk = 0;
    //    ip.ki.wScan = 0x0E8; // è
    //    SendInput(1, &ip, sizeof(INPUT));

    //    // Release key
    //    ip.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    //    SendInput(1, &ip, sizeof(INPUT));
    //}

    ////flip icon
    //options.debug = !options.debug;
    //bool res = ShowInTraybar(options.debug, globalState.recordingMacro >= 0, globalState.activeConfig);
    //if (!res)
    //    cout << endl << "not flipped";
}

bool processOnOffKey()
{
    //handle the @#$ Pause key
    bool pauseKeyTriggeredOnOff = false;
    if (globals.capsicainOnOffKey == VK_CPS_PAUSE)
    {
        //drop all E1 LCTRL
        if (interceptionState.currentIKstroke.state > 3
            && interceptionState.currentIKstroke.code == SC_LCTRL)
        {
            if constexpr (ENABLE_TRACE) cout << endl << "dropping E2 LCTRL";
            return true;
        }

        if (loopState.scancode == SC_NUMLOCK
            && interceptionState.previousIKstroke1.code == SC_LCTRL
            && interceptionState.previousIKstroke1.state > 3)
        {
            pauseKeyTriggeredOnOff = true;
        }
    }

    //toggle ON/OFF ?
    if (loopState.scancode == globals.capsicainOnOffKey || pauseKeyTriggeredOnOff)
    {
        if (loopState.isDownstroke)
        {
            globalState.capsicainOn = !globalState.capsicainOn;
            updateTrayIcon(globalState.capsicainOn, globalState.recordingMacro >= 0, globalState.activeConfig);
            if (globalState.capsicainOn)
            {
                reset();
                cout << endl << endl << "[" << getPrettyVKLabel(globals.capsicainOnOffKey) << "] -> Capsicain ON";
                cout << endl << "active config: " << globalState.activeConfig << " = " << globalState.activeConfigName;
            }
            else
                cout << endl << endl << "[" << getPrettyVKLabel(globals.capsicainOnOffKey) << "] -> Capsicain OFF";
        }
        if constexpr (ENABLE_TRACE) cout << endl << pauseKeyTriggeredOnOff;
        //forward only the three keys that have LEDs, to signal the state of capsicain
        if (globals.capsicainOnOffKey == SC_NUMLOCK
            || globals.capsicainOnOffKey == SC_SCRLOCK
            || globals.capsicainOnOffKey == SC_CAPS)
        {
            if constexpr (ENABLE_TRACE) cout << "OnOff event: setting LED for: " << getPrettyVKLabel(globals.capsicainOnOffKey);
            setLED(globals.capsicainOnOffKey, globalState.capsicainOn);
        }
        return true;
    }

    return false;
}

//handle PRINT, SCRLOCK, PAUSE, NUMLOCK, E1, Exit and Break signals
//return false = drop the key
bool processMessyKeys()
{
    //Alt+Print = ALTPRINT, map to PRINT?
    if (loopState.vcode == SC_ALTPRINT)
    {
        if constexpr (ENABLE_TRACE) cout << endl << SC_ALTPRINT;
        if (globals.translateMessyKeys)
            loopState.vcode = SC_PRINT;
    }

    //Ctrl+NumLock -> pause signal
    if  (globals.protectConsole
            && loopState.vcode == SC_NUMLOCK
            && IS_LCTRL_DOWN
            && IsCapsicainForegroundWindow()
        )
    {
        if (loopState.isDownstroke)
            cout << endl << "INFO: Ctrl+NumLock detected, which is the 'Pause console' signal. Discarding it so capsicain does not freeze.";
        return false;
    }

    //Ctrl+ScrLock -> exit signal
    if  (globals.protectConsole 
            && loopState.vcode == SC_SCRLOCK
            && IS_LCTRL_DOWN
            && IsCapsicainForegroundWindow()
        )
    {
        if (loopState.isDownstroke)
            cout << endl << "INFO: Ctrl+ScrLock detected, which is the 'Exit console' signal. Discarding it so capsicain does not exit.";
        return false;
    }

    //Ctrl+Pause produces SC_BREAK = Exit signal
    if (loopState.vcode == SC_BREAK)
    {
        if constexpr (ENABLE_TRACE) cout << endl << "Ctrl+Pause=BREAK";
        //drop SC_BREAK ?
        if (globals.protectConsole
            && IS_LCTRL_DOWN 
            && IsCapsicainForegroundWindow())
        {
            if (loopState.isDownstroke)
                cout << endl << "INFO: Ctrl+Pause detected, which is the BREAK signal. Discarding it so capsicain does not exit.";
            return false;
        }

        //map break to pause
        if(globals.translateMessyKeys)
            loopState.vcode = VK_CPS_PAUSE;
    }

    //translate unmodified pause key sequence to PAUSE (E1 LCTRL NUMLOCK)
    if (globals.translateMessyKeys)
    {
        if (interceptionState.currentIKstroke.state > 3)
        {
            if (loopState.vcode == SC_LCTRL)
            {
                return false;  //drop the ctrl key
            }
            else
            {
                cout << endl << endl << "??? Extended escape code not handled. What is this key???"
                    << "Please open a ticket on github";
                return false;
            }
        }

        if (interceptionState.previousIKstroke1.state > 3)
        {
            if (interceptionState.previousIKstroke1.code != SC_LCTRL)
            {
                cout << endl << "??? unexpected E1 escape sequence. What kind of key is this?";
                return false;
            }

            if (loopState.vcode == SC_NUMLOCK)
            {
                IFDEBUG if (loopState.isDownstroke)
                    cout << endl << ("INFO: Pause key combo (E1 LCTRL NUMLOCK) -> virtual key PAUSE");
                loopState.vcode = VK_CPS_PAUSE;
            }
        }
    }

    return true;
}

bool testDeviceMask(DEV maskAnd, DEV maskNot, int dev)
{
    if (maskAnd == 0xFFFFFFFF && maskNot == 0)
        return true;
    if (dev < 1 || dev > INTERCEPTION_MAX_DEVICE)
        return false;
    DEV mask = 1 << (dev - 1);
    if ((mask & maskAnd) == mask && (mask & maskNot) == 0)
        return true;
    return false;
}

// Helper: Handle config switching commands (ESC+0 through ESC+9)
void handleConfigSwitch(int scancode)
{
    if (scancode == SC_0)
    {
        cout << endl << "CONFIG CHANGE: " << DISABLED_CONFIG_NUMBER;
        switchConfig(DISABLED_CONFIG_NUMBER, false);
    }
    else if (scancode >= SC_1 && scancode <= SC_9)
    {
        int config = scancode - 1;
        cout << endl << "CONFIG CHANGE: " << config;
        switchConfig(config, false);
    }
}

// processCommand() has been replaced by CommandHandler class
// See commands/CommandHandler.h and commands/CommandHandler.cpp


std::map<uint8_t, Device>* getHardwareId(bool refresh)
{
    if (refresh)
    {
        allMaps.devices.clear();
        for (int i = 0; i <= INTERCEPTION_MAX_DEVICE; ++i)
        {
            wchar_t hardware_id[500] = { 0 };
            string id;
            size_t length = interception_get_hardware_id(interceptionState.interceptionContext, i, hardware_id, sizeof(hardware_id));
            if (length > 0 && length < sizeof(hardware_id))
            {
                //forced conversion will replace special characters > 127 with "?"
                for (wchar_t c : hardware_id)
                {
                    if (c > 127)
                        id += '?';
                    else if (c == 0)
                        break;
                    else
                        id += (char)c;
                }
            } 
            else
                continue;
            id = stringToLower(id);
            allMaps.devices[i] = { id, (bool)interception_is_keyboard(i), (id.find("vid_05ac") != string::npos) || (id.find("vid&000205ac") != string::npos) };
        }
    }
    return &allMaps.devices;
}


bool initConsoleWindow()
{
    //check if already running
    //allow release and debug build at the same time
#ifdef NDEBUG
    CreateMutexA(0, FALSE, "capsicain_release"); // try to create a named mutex
#else
    CreateMutexA(0, FALSE, "capsicain_debug"); // try to create a named mutex
#endif
    if (GetLastError() == ERROR_ALREADY_EXISTS) // did the mutex already exist?
        return false; // quit; mutex is released automatically


    //disable CTRL-C
    SetConsoleCtrlHandler(nullptr, TRUE);

    //disable quick edit; blocking the console window means the keyboard is dead
    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(handle, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode &= ~ENABLE_MOUSE_INPUT;
    SetConsoleMode(handle, mode);

    //colors
    system("color 8E");  //byte1=background, byte2=text
    string title = "Capsicain v" VERSION;
    SetConsoleTitle(title.c_str());
    
    //resize to 800x600
    HWND console = GetConsoleWindow();
    RECT initialRect;
    GetWindowRect(console, &initialRect);
    MoveWindow(console, initialRect.left, initialRect.top, 800, 600, TRUE);

    return true;
}


//reads all GLOBALs from ini, no matter where they are
void parseIniGlobals(capsicain::services::ConfigurationService& configService)
{
    vector<string> sectLines = getTaggedLinesFromIni(INI_TAG_GLOBAL, sanitizedIniContent);
    auto& settings = configService.getGlobalSettingsMutable();
    auto& opts = configService.getOptionsMutable();

    for (string line : sectLines)
    {
        string token = stringCopyFirstToken(line);
        if (token == "debugonstartup")
            opts.debug = true;
        else if (token == "capsicainonoffkey")
        {
            string s = stringGetRestBehindFirstToken(line);
            int key = getVcode(s, PRETTY_VK_LABELS.data());
            if (key < 0)
                cout << "ERROR: unknown key label: " << line << endl;
            else if (key > 255 && key != VK_CPS_PAUSE)
                cout << "ERROR: virtual key makes no sense: " << line << endl;
            else
                settings.capsicainOnOffKey = key;
        }
        else if (token == "iniversion")
            settings.iniVersion = stringGetRestBehindFirstToken(line);
        else if (token == "startminimized")
            settings.startMinimized = true;
        else if (token == "startintraybar")
            settings.startInTraybar = true;
        else if (token == "startahk")
            settings.startAHK = true;
        else if (token == "donttranslatemessykeys")
            settings.translateMessyKeys = false;
        else if (token == "dontprotectconsole")
            settings.protectConsole = false;
        else if ((token == "activeconfigonstartup") || (token == "activelayeronstartup"))
            cout << endl;
        else if (token == "disableesckey")
        {
            auto keys = stringSplit(stringGetRestBehindFirstToken(line), ' ');
            for (auto s : keys)
            {
                int key = getVcode(s, PRETTY_VK_LABELS.data());
                settings.disableEscKey.insert(key);
            }
        }
        else if (token == "forwardesckey")
        {
            auto keys = stringSplit(stringGetRestBehindFirstToken(line), ' ');
            for (auto s : keys)
            {
                int key = getVcode(s, PRETTY_VK_LABELS.data());
                settings.forwardEscKey.insert(key);
            }
        }
        else
            cout << endl << "WARNING: unknown GLOBAL " << token;
    }

    if (!getIntValueForTaggedKey(INI_TAG_GLOBAL, "ActiveConfigOnStartup", settings.activeConfigOnStartup, sanitizedIniContent))
    {
        //backward compat for "layer"
        if (getIntValueForTaggedKey(INI_TAG_GLOBAL, "ActiveLayerOnStartup", settings.activeConfigOnStartup, sanitizedIniContent))
        {
            cout << endl << "INFO: Use 'GLOBAL activeConfigOnStartup' instead of 'GLOBAL activeLayerOnStartup'";
        }
        else
        {
            cout << endl << "No ini setting for 'GLOBAL activeConfigOnStartup'. Setting default config " << settings.activeConfigOnStartup;
        }
    }
}

// Parses the OPTIONS in the given section.
// Returns false if section does not exist.
// Phase 2: Updated to write to ConfigurationService
bool parseIniOptions(std::vector<std::string> assembledIni, capsicain::services::ConfigurationService* configService = nullptr)
{
    vector<string> sectLines = getTaggedLinesFromIni(INI_TAG_OPTIONS, assembledIni);
    globalState.activeConfigName = INI_TAG_OPTIONS+" configName is undefined";

    capsicain::RuntimeOptions* opts = configService ? &configService->getOptionsMutable() : nullptr;

    for (string line : sectLines)
    {
        string token = stringCopyFirstToken(line);
        if (token == "configname")
        {
            globalState.activeConfigName = stringGetRestBehindFirstToken(line);
        }
        else if (token == "layername")  //back compat, deprecated
        {
            globalState.activeConfigName = stringGetRestBehindFirstToken(line);
            cout << endl << "INFO: Option LAYERname is deprecated. Use Option CONFIGname instead.";
        }
        else if (token == "debug")
        {
            if (opts) opts->debug = true;
            options.debug = true;
        }
        else if (token == "flipzy")
        {
            if (opts) opts->flipZY = true;
            options.flipZy = true;
        }
        else if (token == "altalttoalt")
        {
            cout << endl << INI_TAG_OPTIONS+" AltAltToAlt is obsolete. You can do this now with 'REWIRE LALT MOD12 // LALT'";
        }
        else if (token == "flipaltwinonapplekeyboards")
        {
            if (opts) opts->flipAltWinOnAppleKeyboards = true;
            options.flipAltWinOnAppleKeyboards = true;
        }
        else if (token == "lcontrollwinblocksalphamapping")
        {
            if (opts) opts->lControlLWinBlocksAlphaMapping = true;
            options.LControlLWinBlocksAlphaMapping = true;
        }
        else if (token == "processonlyfirstkeyboard")
        {
            if (opts) opts->processOnlyFirstKeyboard = true;
            options.processOnlyFirstKeyboard = true;
        }
        else if (token == "includedeviceid")
        {
            globalState.includeDeviceId = stringGetRestBehindFirstToken(line);
            cout << endl << "INFO: this layer is active for devices whose ID contains '" << globalState.includeDeviceId << "'";
        }
        else if (token == "excludedeviceid")
        {
            globalState.excludeDeviceId = stringGetRestBehindFirstToken(line);
            cout << endl << "INFO: this layer is active for devices whose ID does NOT contain '" << globalState.excludeDeviceId << "'";
        }
        else if (token == "delayforkeysequencems")
        {
            if (opts) getIntValueForKey("delayForKeySequenceMS", opts->delayForKeySequenceMS, sectLines);
            getIntValueForKey("delayForKeySequenceMS", options.delayForKeySequenceMS, sectLines);
        }
        else if (token == "shiftshifttoshiftlock")
        {
            cout << endl << ("WARNING: this is obsolete: OPTION shiftShiftToShiftLock");
            cout << endl << "  Put this into your .ini instead: "
                << endl << "    COMBO  LSHF   [& ....] > key(CAPSOFF)"
                << endl << "    COMBO  RSHF[.&] > key(CAPSON)" << endl;
        }
        else if (token == "holdrepeatsallkeys")
        {
            if (opts) opts->holdRepeatsAllKeys = true;
            options.holdRepeatsAllKeys = true;
        }
        else if (token == "disableahkdelay")
        {
            if (opts) opts->disableAHKDelay = true;
            options.disableAHKDelay = true;
        }
        else if (token == "defaultfunction")
        {
            if (opts) opts->defaultFunction = stringGetRestBehindFirstToken(line);
            options.defaultFunction = stringGetRestBehindFirstToken(line);
        }
        else if (token == "enablemouse")
        {
            if (opts) opts->enableMouse = true;
            options.enableMouse = true;
        }
        else
        {
            cout << endl << "WARNING: ignoring unknown OPTION " << line << endl;
        }
    }

    return true;
}

//fill the rewiremap array
//return # of valid rewires
void parseIniRewires(std::vector<std::string> assembledIni)
{
    vector<string> sectLines = getTaggedLinesFromIni(INI_TAG_REWIRE, assembledIni);

    int tagCounter = 0;
    int keyIn, keyOut, keyTap, keyTapHold;
    for (string line : sectLines)
    {
        keyTap = -1;
        keyTapHold = -1;
        if (parseKeywordRewire(line, keyIn, keyOut, keyTap, keyTapHold, PRETTY_VK_LABELS.data()))
        {
            //duplicate?
            if (allMaps.rewiremap[keyIn][REWIRE_OUT] >= 0)
            {
                cout << endl << "WARNING: ignoring redefinition of " << INI_TAG_REWIRE << " "
                    << PRETTY_VK_LABELS[keyIn] << " " << PRETTY_VK_LABELS[keyOut] << " " << PRETTY_VK_LABELS[keyTap];
                continue;
            }

            if (!isModifier(keyOut) && keyTap > 0)
                cout << endl << "WARNING: 'If-Tapped' definition only makes sense for modifiers: " << INI_TAG_REWIRE << " " << line;

            tagCounter++;
            allMaps.rewiremap[keyIn][REWIRE_OUT] = keyOut;
            allMaps.rewiremap[keyIn][REWIRE_TAP] = keyTap;
            allMaps.rewiremap[keyIn][REWIRE_TAPHOLD] = keyTapHold;
        }
        else
            error("Bad Rewire / key mapping: " + line);
    }
    IFDEBUG cout << endl << "Rewire Definitions: " << dec << tagCounter;
}

bool parseIniCombos(std::vector<std::string> assembledIni)
{
    auto parseSect = [](vector<string>& sectLines, vector<ModifierCombo> &combos) {
        MOD mods[6] = { 0 }; //deadkey, and, or, not, tap, tap/and
        vector<VKeyEvent> keyEventSequence;
        for (string line : sectLines)
        {
            int key;
            DEV devs[2] = { 0xFFFFFFFF, 0 };
            if (parseKeywordCombo(line, key, mods, devs, keyEventSequence, PRETTY_VK_LABELS.data(), options.defaultFunction))
            {
                bool isDuplicate = false;
                for (ModifierCombo testcombo : combos)
                {
                    if (key == testcombo.vkey && devs[0] == testcombo.devAnd && devs[1] == testcombo.devNot && mods[0] == testcombo.deadkey && mods[1] == testcombo.modAnd
                        && mods[2] == testcombo.modOr && mods[3] == testcombo.modNot && mods[4] == testcombo.modTap && mods[5] == testcombo.modTapAnd)
                    {
                        //warn only if the combos are different
                        bool redefined = false;
                        if (testcombo.keyEventSequence.size() == keyEventSequence.size())
                        {
                            for (int i = 0; i < keyEventSequence.size(); i++)
                            {
                                if (keyEventSequence[i].vcode != testcombo.keyEventSequence[i].vcode
                                    || keyEventSequence[i].isDownstroke != testcombo.keyEventSequence[i].isDownstroke)
                                {
                                    redefined = true;
                                    break;
                                }
                            }
                        }
                        else
                            redefined = true;

                        if(redefined)
                            cout << endl << "WARNING: Ignoring redefinition of Combo: " << line;

                        isDuplicate = true;
                        break;
                    }
                }
                if(!isDuplicate)
                    combos.push_back({ key, (unsigned char) mods[0], mods[1], mods[2], mods[3], mods[4], mods[5], devs[0], devs[1], keyEventSequence });
            }
            else
                error("Cannot parse combo rule: " + line);
        }
        return sectLines.size();
    };

    for (auto& kv : allMaps.modCombos)
        kv.second.clear();

    size_t totalLines = 0;
    {
        auto combolines = getTaggedLinesFromIni("COMBO", assembledIni);
        totalLines += parseSect(combolines, allMaps.modCombos[INI_TAG_COMBOS]);
    }
    for (auto& kv : allMaps.modCombos)
    {
        auto lines = getTaggedLinesFromIni(kv.first, assembledIni);
        totalLines += parseSect(lines, kv.second);
    }
    return totalLines > 0;
}

bool parseIniAlphaLayout(std::vector<std::string> assembledIni)
{
    string tagFrom = stringToLower(INI_TAG_ALPHA_FROM);
    string tagEnd = stringToLower(INI_TAG_ALPHA_END);

    string mapFromTo = "";
    bool inMapFromTo = false;
    for (string line : assembledIni)
    {
        string firstToken = stringCopyFirstToken(line);
        if (firstToken == tagFrom)
        {
            if (inMapFromTo)
            {
                error("Bad " + INI_TAG_ALPHA_FROM + ".." + INI_TAG_ALPHA_TO + "definition - received second "+ INI_TAG_ALPHA_FROM +". Forgot the "+INI_TAG_ALPHA_END+"?");
                return false;
            }
            inMapFromTo = true;
            mapFromTo = stringGetRestBehindFirstToken(line) + " ";
        }
        else if (firstToken == tagEnd)
        {
            inMapFromTo = false;
            if (!parseKeywordsAlpha_FromTo(mapFromTo, allMaps.alphamap.data(), PRETTY_VK_LABELS.data()))
                error("Cannot parse the " + INI_TAG_ALPHA_FROM + ".." + INI_TAG_ALPHA_TO + " alpha definition");
        }
        else if (inMapFromTo)
        {
            mapFromTo += line + " ";
        }
    }
    return true;
}

//insert all the INCLUDEd sub-sections into the base config section
std::vector<std::string> assembleConfig(int config)
{
    string sectionName = "config_" + to_string(config);
    vector<string> assembledIni = getSectionFromIni(sectionName, sanitizedIniContent);

    if (assembledIni.size() == 0)
    {
        sectionName = "layer_" + to_string(config);
        assembledIni = getSectionFromIni(sectionName, sanitizedIniContent);

        if (assembledIni.size() > 0)
            cout << endl << "INFO: section [layer_x]  should now be named  [config_x]";
    }

    while (true)
    {
        bool foundInclude = false;
        for (int i = 0; i < assembledIni.size(); i++)
        {
            string line = assembledIni.at(i);
            if (stringStartsWith(line, "include "))
            {
                assembledIni.erase(assembledIni.begin() + i);
                string subSectionName = stringGetRestBehindFirstToken(line);
                vector<string> subsection = getSectionFromIni(subSectionName, sanitizedIniContent);
                if (subsection.size() == 0)
                {
                    error("Subsection [" + subSectionName + "] does not exist or is empty)");
                }
                else
                {
                    IFDEBUG cout << endl << "inserting sub-section: " + subSectionName << " (" << subsection.size() << " lines)";
                    assembledIni.insert(assembledIni.begin() + i, subsection.begin(), subsection.end());
                }
                foundInclude = true;
                break;
            }
        }
        if (!foundInclude)
            break;
    }

    return assembledIni;
}

void parseIniExecutables(std::vector<std::string> assembledIni)
{
    allMaps.executables.clear();
    vector<string> sectLines = getTaggedLinesFromIni(INI_TAG_EXE, assembledIni);
    int tagCounter = 0;
    for (string line : sectLines)
    {
        size_t idIdx = line.find_first_of(' ');
        if (idIdx == string::npos)
        {
            error("Invalid EXE: " + line);
            continue;
        }
        int id;
        if (!stringToInt(line.substr(0, idIdx), id))
        {
            error("Invalid EXE: " + line);
            continue;
        }
        stringstream paramss(line.substr(idIdx + 1));
        string param;
        vector<string> params;
        while(getline(paramss, param, ','))
        {
            ltrim(param);
            rtrim(param);
            params.push_back(param);
        }
        if (params.size() < 2)
        {
            error("Invalid EXE: " + line);
            continue;
        }
        string verb = params[0];
        string path = params[1];
        string args;
        string dir;
        int mode = SW_SHOWDEFAULT;
        if (params.size() > 2)
            args = params[2];
        if (params.size() > 3)
            dir = params[3];
        if (params.size() > 4)
            stringToInt(params[4], mode);

        allMaps.executables[id] = {verb, path, args, dir, mode, nullptr};
        tagCounter++;
    }
    IFDEBUG cout << endl << "Exe    Definitions: " << dec << tagCounter;
}


void initializeAllMaps()
{
    for (auto kv : allMaps.modCombos)
        kv.second.clear();

    allMaps.executables.clear();
    getHardwareId();

    //resetAlphamap()
    {
        for (int i = 0; i < MAX_VCODES; i++)  //initialize to "map to same char"
            allMaps.alphamap[i] = i;
    }

    //resetRewiremap()
    {
        for (int r = 0; r < REWIRE_ROWS; r++)
            for (int c = 0; c < REWIRE_COLS; c++)
                allMaps.rewiremap[r][c] = -1;
    }
}


//processes the sanitized ini that was read on startup or reload
bool parseProcessIniConfig(int config)
{
    initializeAllMaps();

    if (sanitizedIniContent.size() == 0)
    {
        cout << endl << "Capsicain.ini is missing or empty.";
        return false;
    }

    vector<string> assembledConfig = assembleConfig(config);
    if (assembledConfig.size() == 0)
    {
        cout << endl << "No valid configuration for Config " << config;
        return false;
    }

    IFDEBUG cout << endl << "Assembled config #" << config << " : " << dec << assembledConfig.size() << " lines";

    parseIniOptions(assembledConfig);

    parseIniRewires(assembledConfig);

    parseIniExecutables(assembledConfig);

    parseIniCombos(assembledConfig);

    // Convert legacy combos to domain ComboRules once (for performance)
    for (auto& pair : allMaps.modCombos) {
        const string& tag = pair.first;
        const vector<ModifierCombo>& legacyCombos = pair.second;
        vector<capsicain::domain::ComboRule>& convertedRules = allMaps.convertedCombos[tag];

        convertedRules.clear();
        convertedRules.reserve(legacyCombos.size());

        for (const auto& combo : legacyCombos) {
            capsicain::domain::ComboRule rule;
            rule.triggerKey = combo.vkey;
            rule.modAnd = combo.modAnd;
            rule.modOr = combo.modOr;
            rule.modNot = combo.modNot;
            rule.modTap = combo.modTap;
            rule.modTapAnd = combo.modTapAnd;
            rule.devAnd = combo.devAnd;
            rule.devNot = combo.devNot;
            rule.deadkey = combo.deadkey;
            rule.resultSequence.reserve(combo.keyEventSequence.size());
            for (const auto& evt : combo.keyEventSequence) {
                rule.resultSequence.push_back({
                    static_cast<capsicain::domain::VKeyCode>(evt.vcode),
                    evt.isDownstroke
                });
            }
            convertedRules.push_back(std::move(rule));
        }
    }

    IFDEBUG cout << endl << "Down   Definitions: " << dec << allMaps.modCombos[INI_TAG_COMBOS].size();
    IFDEBUG cout << endl << "Up     Definitions: " << dec << allMaps.modCombos[INI_TAG_UPCOMBOS].size();
    IFDEBUG cout << endl << "Tap    Definitions: " << dec << allMaps.modCombos[INI_TAG_TAPCOMBOS].size();
    IFDEBUG cout << endl << "Slow   Definitions: " << dec << allMaps.modCombos[INI_TAG_SLOWCOMBOS].size();
    IFDEBUG cout << endl << "Repeat Definitions: " << dec << allMaps.modCombos[INI_TAG_REPEATCOMBOS].size();

    parseIniAlphaLayout(assembledConfig);
    IFDEBUG
    {
        int remapped = 0;
        for (int i = 0; i < MAX_VCODES; i++)
            if (i != allMaps.alphamap[i])
                remapped++;
        cout << endl << "Alpha  Definitions: " << dec << remapped;
    }

    return true;
}

void switchConfig(int config, bool forceReloadSameConfig)
{
    if (!forceReloadSameConfig && config == globalState.activeConfig)
        return;

    int oldConfig = globalState.activeConfig;
    reset();

    if (config == DISABLED_CONFIG_NUMBER)
    {
        globalState.activeConfig = DISABLED_CONFIG_NUMBER;
        globalState.activeConfigName = DISABLED_CONFIG_NAME;
    }
    else if (parseProcessIniConfig(config))
    {
        globalState.activeConfig = config;
        globalState.previousConfig = oldConfig;
        printOptions();
    }
    else if (parseProcessIniConfig(oldConfig))
    {
        cout << endl << endl << "Keeping the current config";
    }
    else
    {
        cout << endl << endl << "ERROR: CANNOT RELOAD CURRENT CONFIG? Switching to config 0";
        globalState.activeConfig = DISABLED_CONFIG_NUMBER;
        globalState.activeConfigName = DISABLED_CONFIG_NAME;
    }

    if (interceptionState.interceptionContext)
    {
        if (options.enableMouse)
            interception_set_filter(interceptionState.interceptionContext, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_ALL & ~INTERCEPTION_FILTER_MOUSE_MOVE);
        else
            interception_set_filter(interceptionState.interceptionContext, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_NONE);
    }

    updateTrayIcon(true, globalState.recordingMacro >= 0, globalState.activeConfig);
    cout << endl << endl << "ACTIVE CONFIG: " << globalState.activeConfig << " = " << globalState.activeConfigName;
}

void resetCapsNumScrollLock()
{ 
    //set NumLock, release CapsLock+Scrolllock
    vector<VKeyEvent> sequence;
    if (!(GetKeyState(VK_NUMLOCK) & 0x0001))
        keySequenceAppendMakeBreakKey(SC_NUMLOCK, sequence);
    if (GetKeyState(VK_CAPITAL) & 0x0001)
        keySequenceAppendMakeBreakKey(SC_CAPS, sequence);
    if (GetKeyState(VK_SCROLL) & 0x0001 && globals.capsicainOnOffKey != SC_SCRLOCK)  //don't mess with ScrLock when it is the enable/disable key
        keySequenceAppendMakeBreakKey(SC_SCRLOCK, sequence);
    if (sequence.size() != 0)
        playKeyEventSequence(sequence);
}

void reset()
{
    releaseAllSentKeys();

    loopState = defaultLoopState;
    modifierState = defaultModifierState;

    if constexpr (ENABLE_PROFILING)
    {
        chrono::steady_clock::time_point tps = profiler.timepointStopwatch;
        chrono::steady_clock::time_point tppk = profiler.timepointPreviousKeyEvent;
        chrono::steady_clock::time_point tpls = profiler.timepointLoopStart;
        profiler = defaultProfiler;
        profiler.timepointStopwatch = tps;
        profiler.timepointPreviousKeyEvent = tppk;
        profiler.timepointLoopStart = tpls;
    }

    GlobalState tmp = globalState; //some settings shall survive the reset
    globalState = defaultGlobalState;
    globalState.deviceIdKeyboard = tmp.deviceIdKeyboard;
    globalState.activeConfig = tmp.activeConfig;
    globalState.activeConfigName = tmp.activeConfigName;
    globalState.previousConfig = tmp.previousConfig;
    for(int i=0;i<MAX_NUM_MACROS;i++)
        globalState.recordedMacros[i] = tmp.recordedMacros[i];
}

//Reset and reload the ini from scratch
void reload(capsicain::services::ConfigurationService& configService)
{
    initializeAllMaps();
    globals = defaultGlobals;
    options = defaultOptions;

    readSanitizeIniFile(sanitizedIniContent);

    parseIniGlobals(configService);

    // Copy GlobalSettings to legacy globals struct
    const auto& settings = configService.getGlobalSettings();
    globals.iniVersion = settings.iniVersion;
    globals.activeConfigOnStartup = settings.activeConfigOnStartup;
    globals.startMinimized = settings.startMinimized;
    globals.startInTraybar = settings.startInTraybar;
    globals.startAHK = settings.startAHK;
    globals.capsicainOnOffKey = settings.capsicainOnOffKey;
    globals.protectConsole = settings.protectConsole;
    globals.translateMessyKeys = settings.translateMessyKeys;
    globals.disableEscKey = settings.disableEscKey;
    globals.forwardEscKey = settings.forwardEscKey;

    // Copy RuntimeOptions to legacy options struct
    const auto& opts = configService.getOptions();
    options.debug = opts.debug;
    options.delayForKeySequenceMS = opts.delayForKeySequenceMS;
    options.flipZy = opts.flipZY;
    options.flipAltWinOnAppleKeyboards = opts.flipAltWinOnAppleKeyboards;
    options.LControlLWinBlocksAlphaMapping = opts.lControlLWinBlocksAlphaMapping;
    options.processOnlyFirstKeyboard = opts.processOnlyFirstKeyboard;
    options.holdRepeatsAllKeys = opts.holdRepeatsAllKeys;
    options.disableAHKDelay = opts.disableAHKDelay;
    options.defaultFunction = opts.defaultFunction;
    options.enableMouse = opts.enableMouse;

    if (globals.startAHK)
        loadAHK();
    else
        unloadAHK();

    switchConfig(globalState.activeConfig, true);
}

//Release all keys to 'up' that have been sent out as 'down'
void releaseAllSentKeys()
{
    IFDEBUG cout << endl << "Resetting all sent DOWN keys to UP: " << endl;

    modifierState.modifierForceDown = 0;

    // release backwards to release modifiers and esc last
    for (int i = 255; i >= 0; --i)
    {
        globalState.holdKeys[i].clear();
        if (globalState.keysDownSent[i])
        {
            sendVKeyEvent({ i, false });
        }
    }
}


//================================================================
// Console UI Functions - Thin wrappers for ConsoleUI class
// These exist for compatibility with legacy code
//================================================================

void printOptions()
{
    if (g_uiService) g_uiService->printOptions();
}

void printStatus()
{
    if (g_uiService) g_uiService->getConsoleUI().printStatus();
}

void printHelp()
{
    if (g_uiService) g_uiService->printHelp();
}

void printKeylabels()
{
    if (g_uiService) g_uiService->printKeylabels();
}

void normalizeIKStroke(InterceptionKeyStroke &ikstroke) {
    if (ikstroke.code > 0x7F) {
        ikstroke.code &= 0x7F;
        ikstroke.state |= 2;
    }
}

InterceptionKeyStroke convertVkeyEvent2ikstroke(VKeyEvent vkstroke)
{
    InterceptionKeyStroke iks = { (unsigned short) vkstroke.vcode, 0 };

    if (vkstroke.vcode >= 0xFF)
    {
        error("BUG: trying to send an interception keystroke > xFF");
        iks.code = SC_NOP;
    }

    if (vkstroke.vcode >= 0x80)
    {
        iks.code = static_cast<unsigned short>(vkstroke.vcode & 0x7F);
        iks.state |= 2;
    }
    if (!vkstroke.isDownstroke)
        iks.state |= 1;

    return iks;
}

VKeyEvent convertIkstroke2VKeyEvent(InterceptionKeyStroke ikStroke)
{
    VKeyEvent strk;
    strk.vcode = ikStroke.code;
    if ((ikStroke.state & 2) == 2)
        strk.vcode |= 0x80;
    strk.isDownstroke = ikStroke.state & 1 ? false : true;
    return strk;
}

//handle all special Capsicain VCodes that have no "second key param". Trigger events on downstroke only
void sendCapsicainCodeHandler(VKeyEvent keyEvent)
{
    if (!keyEvent.isDownstroke)
        return;

    if constexpr (ENABLE_TRACE) cout << endl << "(CPS code: " << getPrettyVKLabelPadded(keyEvent.vcode, 0) << ")";

    switch (keyEvent.vcode)
    {
    case VK_CPS_CAPSON:
    {
        if (!(GetKeyState(VK_CAPITAL) & 0x0001))
        {
            sendVKeyEvent({ SC_CAPS, true });
            sendVKeyEvent({ SC_CAPS, false });
        }
        break;
    }
    case VK_CPS_CAPSOFF:
    {
        if ((GetKeyState(VK_CAPITAL) & 0x0001))
        {
            sendVKeyEvent({ SC_CAPS, true });
            sendVKeyEvent({ SC_CAPS, false });
        }
        break;
    }
    case VK_CPS_CONFIGPREVIOUS:
    {
        switchConfig(globalState.previousConfig, false);
        break;
    }
    case VK_CPS_OBFUSCATED_SEQUENCE_START:
    {
        globalState.secretSequencePlayback = true;
        break;
    }
    case VK_CPS_PAUSE:
        if (globals.protectConsole && IsCapsicainForegroundWindow())
        {
            cout << endl << endl << "INFO: Discarding the PAUSE key. " << endl 
                << "      This would freeze Capsicain which is currently the active window (and this would stop your keyboard)";
            break;
        }
        
        //manually send a PAUSE sequence with E1 escape (iks state 4/5)
        if constexpr (ENABLE_TRACE) cout << endl << "sending the Pause key sequence E1 LCTRL NUMLOCK";
        InterceptionKeyStroke iks_cont = {SC_LCTRL,4,0};
        interception_send(interceptionState.interceptionContext, interceptionState.interceptionDevice, (InterceptionStroke*)&iks_cont, 1);
        InterceptionKeyStroke iks_numl = { SC_NUMLOCK,0,0 };
        interception_send(interceptionState.interceptionContext, interceptionState.interceptionDevice, (InterceptionStroke*)&iks_numl, 1);
        iks_cont.state = 5;
        interception_send(interceptionState.interceptionContext, interceptionState.interceptionDevice, (InterceptionStroke*)&iks_cont, 1);
        iks_numl.state = 1;
        interception_send(interceptionState.interceptionContext, interceptionState.interceptionDevice, (InterceptionStroke*)&iks_numl, 1);

        break;
    }
}

void sendResultingKeyOrSequence()
{
    if (loopState.resultingVKeyEventSequence.size() > 0)
    {
        playKeyEventSequence(loopState.resultingVKeyEventSequence);
    }
    else
    {
        IFDEBUG
        {
            if (loopState.scancode != loopState.vcode)
                cout << "  --  " << PRETTY_VK_LABELS[loopState.vcode] << getSymbolForIKStrokeState(interceptionState.currentIKstroke.state);
            else
                cout << "  -->";
        }
        {
            sendVKeyEvent({ loopState.vcode, loopState.isDownstroke });
        }
    }
}

bool runExecutable(Executable &exe)
{
    char path[MAX_PATH];
    char args[MAX_PATH];
    char dir[MAX_PATH];
    ZeroMemory(path, MAX_PATH);
    ZeroMemory(args, MAX_PATH);
    ZeroMemory(dir, MAX_PATH);
    ExpandEnvironmentStringsA(exe.path.c_str(), path, MAX_PATH);
    ExpandEnvironmentStringsA(exe.args.c_str(), args, MAX_PATH);
    ExpandEnvironmentStringsA(exe.dir.c_str(), dir, MAX_PATH);

    SHELLEXECUTEINFOA info = {0};
    info.cbSize = sizeof(SHELLEXECUTEINFO);
    info.fMask = SEE_MASK_NO_CONSOLE | SEE_MASK_NOCLOSEPROCESS;
    info.lpVerb = exe.verb.c_str();
    info.lpFile = path;
    info.lpParameters = args;
    info.lpDirectory = dir;
    info.nShow = exe.mode;
    ShellExecuteExA(&info);
    exe.proc = info.hProcess;
    exe.pid = GetProcessId(exe.proc);
    exe.proc = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, exe.pid);
    auto ret = (INT_PTR)info.hInstApp > 32;
    return ret;
}

void sendAHK(const std::string& msg)
{
    if (g_ahkService) {
        g_ahkService->send(msg);
    }
}

void killExecutableByPath(string path)
{
    auto slash = path.find_last_of("\\/");
    if (slash != string::npos)
        path = path.substr(slash + 1);
    string ext = ".exe";
    if (!std::equal(ext.rbegin(), ext.rend(), path.rbegin()))
        path = path + ext;
    closeOrKillProgram(path);
}

void killExecutable(Executable &exe)
{
    if (exe.pid && GetProcessId(exe.proc) == exe.pid)
    {
        HANDLE hProc = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, exe.pid);
        EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM)exe.pid);
        int result = 1; // 0=fail; 1=close; 2=kill
        if (WaitForSingleObject(hProc, 1000) != WAIT_OBJECT_0)
            result = (TerminateProcess(hProc, 0) ? 2 : 0);
        CloseHandle(hProc);
    }
    else
    {
        killExecutableByPath(exe.path);
    }
    exe.proc = 0;
    exe.pid = 0;
    exe.hwnd = 0;
}

//Send out all keys in a sequence
//Sequences are created for anything that requires more than one key event, like AltChar(123)
//Catch and process CPS virtual keys that have a value following in the next key
void playKeyEventSequence(vector<VKeyEvent> keyEventSequence)
{
    if (keyEventSequence.size() == 0) 
    {
        cout << endl << "BUG? keyEventSequence.size == 0" << endl;
        return;
    }

    VKeyEvent newKeyEvent;
    unsigned int delayBetweenKeyEventsMS = options.delayForKeySequenceMS;
    bool tempReleasedKeys = false; //command to temporarily release all physical keys that came before the current combo

    //remember that the next key will be the value for a func key'
    int  expectParamForFuncKey = -1;

    IFDEBUG
        if (!globalState.secretSequencePlayback && keyEventSequence.at(0).vcode != VK_CPS_OBFUSCATED_SEQUENCE_START)
             cout << "  --> SEQUENCE (" << dec << keyEventSequence.size() << ") ";

    for (VKeyEvent keyEvent : keyEventSequence)
    {
        // can be changed during key sequence by delay()
        delayBetweenKeyEventsMS = options.delayForKeySequenceMS;
        int vc = keyEvent.vcode;
        if (globalState.secretSequencePlayback)
            vc = deObfuscateVKey(vc);

        //test if this is the param for the preceding func key in "command + value" sequence
        if (expectParamForFuncKey != -1)
        {
            IFDEBUG cout << "{" + PRETTY_VK_LABELS[expectParamForFuncKey] + "}";
            switch (expectParamForFuncKey)
            {
            case VK_CPS_SLEEP:
                if constexpr (ENABLE_TRACE) cout << endl << "vk_cps_sleep: " << vc;
                Sleep(vc);
                break;
            case VK_CPS_DEADKEY:
                if constexpr (ENABLE_TRACE) cout << endl << "vk_cps_deadkey: " << getPrettyVKLabelPadded(vc, 0);
                modifierState.activeDeadkey = vc;
                break;
            case VK_CPS_CONFIGSWITCH:
                if constexpr (ENABLE_TRACE) cout << endl << "vk_cps_configswitch: " << vc;
                switchConfig(vc, false);
                break;
            case VK_CPS_RECORDMACRO:
            case VK_CPS_RECORDSECRETMACRO:
            {
                int macroNum = vc;

                bool isSecret = false;
                if (expectParamForFuncKey == VK_CPS_RECORDSECRETMACRO)
                    isSecret = true;

                if (macroNum < 1 || macroNum >= MAX_NUM_MACROS)
                    cout << endl << "ERROR in .ini: bad number for macro. Must be 1.." << MAX_NUM_MACROS - 1;
                else if (globalState.recordingMacro != -1)
                    cout << endl << "INFO: a macro is already being recorded: #" << globalState.recordingMacro;
                else
                {
                    IFDEBUG cout << endl << "Start recording " << (isSecret ? "secret" : "") << "macro #" << macroNum << endl;
                    globalState.recordingMacro = macroNum;
                    globalState.recordedMacros[macroNum].clear();

                    if (isSecret)
                    {
                        globalState.secretSequenceRecording = true;
                        globalState.recordedMacros[macroNum].push_back({ VK_CPS_OBFUSCATED_SEQUENCE_START, true });
                    }
                }
                updateTrayIcon(true, globalState.recordingMacro >= 0, globalState.activeConfig);
                break;
            }
            case VK_CPS_PLAYMACRO:
            {
                if constexpr (ENABLE_TRACE) cout << endl << "vk_cps_playmacro: " << vc;
                int macnum = vc;

                if (macnum < 1 || macnum >= MAX_NUM_MACROS)
                    cout << endl << "ERROR: bad number for macro. Must be 1.." << MAX_NUM_MACROS - 1;
                else
                {
                    if (globalState.recordedMacros[macnum].size() == 0)
                        cout << endl << "INFO macro #" << macnum << " has not been recorded before.";
                    else
                    {
                        playKeyEventSequence(globalState.recordedMacros[macnum]);
                        globalState.secretSequencePlayback = false;
                    }
                }
                break;
            }
            case VK_CPS_HOLDKEY:
            {
                if constexpr (ENABLE_TRACE) cout << endl << "vk_cps_holdkey: " << getPrettyVKLabelPadded(loopState.vcode, 0) << " -> " << getPrettyVKLabelPadded(vc, 0);
                if (!getKeyHolding(vc))
                {
                    globalState.holdKeys[loopState.vcode].emplace(vc);
                    sendVKeyEvent(keyEvent, false);
                }
                else
                    sendVKeyEvent(keyEvent);
                break;
            }
            case VK_CPS_HOLDMOD:
            {
                if (!getKeyHolding(vc))
                {
                    if (modifierState.modifierDown)
                    {
                        for (int i = 0; i < NUMBER_OF_MODIFIERS; i++)
                        {
                            MOD mask = 1 << i;
                            if (modifierState.modifierDown & mask)
                            {
                                int mod = getModifierForBitmask(mask);
                                if constexpr (ENABLE_TRACE) cout << endl << "vk_cps_holdmod: " << getPrettyVKLabelPadded(mod, 0) << " -> " << getPrettyVKLabelPadded(vc, 0);
                                globalState.holdKeys[mod].emplace(vc);
                                break;
                            }
                        }
                    }
                    else
                    {
                        globalState.holdKeys[loopState.vcode].emplace(vc);
                    }
                    sendVKeyEvent(keyEvent, false);
                }
                else
                    sendVKeyEvent(keyEvent);
                break;
            }
            case VK_CPS_DELAY:
                if constexpr (ENABLE_TRACE) cout << endl << "vk_cps_delay: " << vc;
                options.delayForKeySequenceMS = vc;
                break;
            case VK_CPS_KEYDOWN:
                if (vc < 0xFF)
                    sendVKeyEvent(keyEvent);
                if (isModifier(vc))
                {
                    if (keyEvent.isDownstroke)
                    {
                        modifierState.modifierForceDown |= getModifierBitmaskForVcode(vc);
                        modifierState.modifierDown |= modifierState.modifierForceDown;
                    }
                    else
                    {
                        modifierState.modifierForceDown &= ~getModifierBitmaskForVcode(vc);
                        modifierState.modifierDown &= modifierState.modifierForceDown;
                    }
                }
                break;
            case VK_CPS_KEYTOGGLE:
                bool state;
                if (isModifier(vc))
                {
                    auto mask = getModifierBitmaskForVcode(vc);
                    state = modifierState.modifierForceDown & mask;
                    modifierState.modifierForceDown ^= mask;
                    modifierState.modifierDown &= modifierState.modifierForceDown;
                }
                else
                {
                    state = globalState.keysDownSent[vc & 0xFF];
                }
                if (vc < 0xFF)
                    sendVKeyEvent({vc, !state});
                break;
            case VK_CPS_KEYTAP:
                if (!isModifier(vc))
                    break;
                if (keyEvent.isDownstroke)
                    modifierState.modifierTapped |= getModifierBitmaskForVcode(vc);
                else
                    modifierState.modifierTapped &= ~getModifierBitmaskForVcode(vc);
                break;
            case VK_CPS_EXECUTE:
            {
                if (allMaps.executables.find(vc) == allMaps.executables.end())
                {
                    IFDEBUG cout << "Can't find executable " << vc << endl;
                    break;
                }
                runExecutable(allMaps.executables[vc]);
                break;
            }
            case VK_CPS_KILL:
            {
                if (allMaps.executables.find(vc) == allMaps.executables.end())
                {
                    IFDEBUG cout << "Can't find executable " << vc << endl;
                    break;
                }
                killExecutable(allMaps.executables[vc]);
                break;
            }
            case VK_CPS_SENDAHK:
            {
                auto msg = getAHKmsg(vc);
                if (msg != "")
                    sendAHK(msg);
                break;
            }
            default:
                cout << endl << "BUG? unknown expectParamForFuncKey";
            }

            expectParamForFuncKey = -1;
            continue;
        }

        //in no special state, evaluate the key
        if (vc == VK_CPS_TEMPRELEASEKEYS) //release and remember all keys that are physically down
        {
            bool tempReleasedKeys = true;
            for (int i = 0; i <= 255; i++)
            {
                globalState.keysDownTempReleased[i] = globalState.keysDownSent[i];
                if (globalState.keysDownSent[i])
                    sendVKeyEvent({ i, false });
            }
            if (globalState.keysDownSentCounter != 0)
                error("BUG: keysDownSentCounter != 0");
        }
        else if (vc == VK_CPS_TEMPRESTOREKEYS) //restore all keys that were down before 'VK_cps_temprelease'
        {
            bool tempReleasedKeys = false;
            for (int i = 0; i <= 255; i++)
            {
                if (globalState.keysDownTempReleased[i])
                {
                    sendVKeyEvent({ i, true });
                    globalState.keysDownTempReleased[i] = false;
                }
            }
        }
        else if (vc == VK_CPS_RELEASEKEYS) //release all keys that are physically down
        {
            for (int i = 0; i <= 255; i++)
                if (globalState.keysDownSent[i])
                    sendVKeyEvent({ i, false }, false);
        }
        //func key with param; wait for next key which is the param
        else if (vc == VK_CPS_SLEEP
            || vc == VK_CPS_DEADKEY
            || vc == VK_CPS_CONFIGSWITCH
            || vc == VK_CPS_RECORDMACRO
            || vc == VK_CPS_RECORDSECRETMACRO
            || vc == VK_CPS_PLAYMACRO
            || vc == VK_CPS_HOLDKEY
            || vc == VK_CPS_HOLDMOD
            || vc == VK_CPS_DELAY
            || vc == VK_CPS_KEYDOWN
            || vc == VK_CPS_KEYTOGGLE
            || vc == VK_CPS_KEYTAP
            || vc == VK_CPS_EXECUTE
            || vc == VK_CPS_KILL
            || vc == VK_CPS_SENDAHK
            )
        {
            expectParamForFuncKey = vc;
        }
        else //regular non-escaped keyEvent
        {
            if(globalState.secretSequencePlayback)
                sendVKeyEvent({ deObfuscateVKey(keyEvent.vcode) , keyEvent.isDownstroke });
            else
                sendVKeyEvent(keyEvent);
            if (!options.disableAHKDelay && (vc == AHK_HOTKEY1 || vc == AHK_HOTKEY2))
                Sleep(DEFAULT_DELAY_FOR_AHK_MS);
            else
                Sleep(delayBetweenKeyEventsMS);
        }
    }

    if (tempReleasedKeys)
        error("VK_CPS_TEMPRELEASEKEYS without corresponding VK_CPS_TEMPRESTOREKEYS. Check your config.");
    if (expectParamForFuncKey != -1)
        error("BUG: func key with param: " + getPrettyVKLabel(expectParamForFuncKey) + "is unfinished");
}

int getKeyHolding(int vcode)
{
    for (int i = 0; i < VK_MAX; i++)
    {
        if (globalState.holdKeys[i].find(vcode) != globalState.holdKeys[i].end())
            return i;
    }
    return 0;
}

std::string getHoldKeyString(std::set<int> &v, std::string delim)
{
    std::string out;
    for (auto it = v.rbegin(); it != v.rend(); ++it)
    {
        out += PRETTY_VK_LABELS[*it];
        out += delim;
    }
    for (int i = 0; i < delim.size(); ++i)
        out.pop_back();
    return out;
}

map<int, int> KEY_TO_MOUSE{
    { VM_LEFT, INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN },
    { VM_RIGHT, INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN },
    { VM_MIDDLE, INTERCEPTION_MOUSE_MIDDLE_BUTTON_DOWN },
    { VM_BUTTON4, INTERCEPTION_MOUSE_BUTTON_4_DOWN },
    { VM_BUTTON5, INTERCEPTION_MOUSE_BUTTON_5_DOWN },
};
bool vkeyToMouse(VKeyEvent keyEvent)
{
    if (keyEvent.vcode < VM_LEFT || keyEvent.vcode > VM_WHEEL_RIGHT)
        return false;

    InterceptionMouseStroke mstroke{0};

    if (keyEvent.vcode >= VM_LEFT && keyEvent.vcode <= VM_BUTTON5)
    {
        mstroke.state = KEY_TO_MOUSE[keyEvent.vcode];
        if (!keyEvent.isDownstroke)
            mstroke.state = mstroke.state << 1;
    }
    else if (keyEvent.vcode == VM_WHEEL_UP)
    {
        mstroke.state = INTERCEPTION_MOUSE_WHEEL;
        mstroke.rolling = 120; //FIXME
    }
    else if (keyEvent.vcode == VM_WHEEL_DOWN)
    {
        mstroke.state = INTERCEPTION_MOUSE_WHEEL;
        mstroke.rolling = -120; //FIXME
    }
    else if (keyEvent.vcode == VM_WHEEL_LEFT)
    {
        mstroke.state = INTERCEPTION_MOUSE_HWHEEL;
        mstroke.rolling = -120; //FIXME
    }
    else if (keyEvent.vcode == VM_WHEEL_RIGHT)
    {
        mstroke.state = INTERCEPTION_MOUSE_HWHEEL;
        mstroke.rolling = 120; //FIXME
    }
    else
    {
        return false;
    }

    if (mstroke.rolling && !keyEvent.isDownstroke)
        return true;

    int dev;
    if (interception_is_mouse(interceptionState.interceptionDevice))
    {
        dev = interceptionState.interceptionDevice;
    }
    else if(interceptionState.lastMouse)
    {
        dev = interceptionState.lastMouse;
    }
    else
    {
        cout << endl << "Error: Don't know which mouse to send this to, use one!";
        return false;
    }

    interception_send(interceptionState.interceptionContext, dev, (InterceptionStroke *)&mstroke, 1);
    return true;
}

void sendVKeyEvent(VKeyEvent keyEvent, bool hold)
{
    if constexpr (ENABLE_TRACE) cout << endl << "sendVkeyEvent(" << keyEvent.vcode << ")";
    if (keyEvent.vcode < 0)
    {
        cout << endl << "BUG: vcode<0";
        return;
    }

    if (keyEvent.vcode == 0)
    {
        IFDEBUG cout << endl << "{blocked NOP}";
        return;
    }

    if (globalState.holdKeys[keyEvent.vcode].size() && hold)
    {
        int code = keyEvent.vcode;
        set<int> release;
        IFDEBUG cout << " {" << PRETTY_VK_LABELS[code] << (keyEvent.isDownstroke ? " holding " : " released ") << globalState.holdKeys[code].size() << ": " << getHoldKeyString(globalState.holdKeys[code], "+") << "}";
        if (keyEvent.isDownstroke)
        {
            if (options.holdRepeatsAllKeys)
            {
                for (auto it = globalState.holdKeys[code].begin(); it != globalState.holdKeys[code].end(); ++it)
                    sendVKeyEvent({*it, true}, false);
            }
            else
            {
                sendVKeyEvent({*globalState.holdKeys[code].begin(), true}, false);
            }
            return;
        }
        else
        {
            for (auto it = globalState.holdKeys[code].rbegin(); it != globalState.holdKeys[code].rend(); ++it)
                release.emplace(*it);
            globalState.holdKeys[code].clear();
            for (auto key : release)
                sendVKeyEvent({key, false}, false);
            if (release.find(keyEvent.vcode) != release.end())
                return;
        }
    }

    if (keyEvent.vcode > 0xFF || keyEvent.vcode == VK_CPS_PAUSE)
    {
        sendCapsicainCodeHandler(keyEvent);
        return;
    }

    unsigned char scancode = (unsigned char) keyEvent.vcode;

    if (scancode == 0xE4)  //what was that for?
        IFDEBUG cout << " {sending E4} ";

    if (!keyEvent.isDownstroke &&  !globalState.keysDownSent[scancode])  //ignore up when key is already up
    {
        IFDEBUG cout << " {blocked " << PRETTY_VK_LABELS[scancode] << " UP: was not down}";
        return;
    }

    auto holdingkey = getKeyHolding(scancode);
    if (!keyEvent.isDownstroke && holdingkey)  //ignore up when other key is holding it
    {
        IFDEBUG cout << " {blocked " << PRETTY_VK_LABELS[scancode] << " UP: " << PRETTY_VK_LABELS[holdingkey] << " is holding}";
        return;
    }

    //consistency check
    if (scancode < VM_WHEEL_UP)
    {
        if (globalState.keysDownSent[scancode] == 0 && keyEvent.isDownstroke)
            globalState.keysDownSentCounter++;
        else if (globalState.keysDownSent[scancode] == 1 && !keyEvent.isDownstroke)
            globalState.keysDownSentCounter--;

            globalState.keysDownSent[scancode] = keyEvent.isDownstroke;
    }

    //handle live macro recording
    if (globalState.recordingMacro >= 0)
    {
        if (globalState.recordedMacros[globalState.recordingMacro].size() >= MAX_MACRO_LENGTH -2)  //macro getting too big
        {
            globalState.recordingMacro = -1;
            globalState.secretSequenceRecording = false;
            updateTrayIcon(true, globalState.recordingMacro >= 0, globalState.activeConfig);
            cout << endl << endl << "Macro Length > " << MAX_MACRO_LENGTH << ". Forgotten Macro?" << "Stop recording macro #" << globalState.recordingMacro << endl << endl;
        }
        else
        { 
            //drop upstroke from the starting shortcut?
            if (keyEvent.isDownstroke || globalState.recordedMacros[globalState.recordingMacro].size() > 0 )
            {
                //store the macro obfuscated?
                VKeyEvent obfusc = keyEvent;
                if (globalState.secretSequenceRecording)
                    obfusc.vcode = obfuscateVKey(obfusc.vcode);
                globalState.recordedMacros[globalState.recordingMacro].push_back(obfusc);
            }
        }
    }

    if (vkeyToMouse(keyEvent))
        return;

    InterceptionKeyStroke iks = convertVkeyEvent2ikstroke(keyEvent);
    //hide secret macro recording?
    IFDEBUG
        if(!globalState.secretSequencePlayback)
            cout << " {" << PRETTY_VK_LABELS[keyEvent.vcode] << (keyEvent.isDownstroke ? "v" : "^") << " #" << globalState.keysDownSentCounter << "}";

    int dev;
    if (interception_is_keyboard(interceptionState.interceptionDevice))
    {
        dev = interceptionState.interceptionDevice;
    }
    else if(interceptionState.lastKeyboard)
    {
        dev = interceptionState.lastKeyboard;
    }
    else
    {
        cout << endl << "Don't know which keyboard to send this to, use one!";
        return;
    }

    interception_send(interceptionState.interceptionContext, dev, (InterceptionStroke *)&iks, 1);

    //restore LEDs for ON/OFF indication?
    if (globals.capsicainOnOffKey >0 
        && keyEvent.isDownstroke
        && (globals.capsicainOnOffKey == SC_NUMLOCK || globals.capsicainOnOffKey == SC_SCRLOCK || globals.capsicainOnOffKey == SC_CAPS)
        && (keyEvent.vcode == SC_NUMLOCK || keyEvent.vcode == SC_SCRLOCK || keyEvent.vcode == SC_CAPS ) 
        //does ESC reset ScrLock on some KBs? In that case re-enable ESC check  || keyEvent.vcode == SC_ESCAPE)
        )
    {
        Sleep(50); //give Windows time to register e.g. NumLock key event, since soon we will query its state
        setLED(globals.capsicainOnOffKey, true);
    }
}


void keySequenceAppendMakeKey(unsigned short scancode, vector<VKeyEvent> &sequence)
{
    sequence.push_back({ scancode, true });
}
void keySequenceAppendBreakKey(unsigned short scancode, vector<VKeyEvent> &sequence)
{
    sequence.push_back({ scancode, false });
}
void keySequenceAppendMakeBreakKey(unsigned short scancode, vector<VKeyEvent> &sequence)
{
    sequence.push_back({ scancode, true });
    sequence.push_back({ scancode, false });
}

string getSymbolForIKStrokeState(unsigned short state)
{
    switch (state)
    {
    case 0b000: return "v";
    case 0b001: return "^";
    case 0b010: return "v*";
    case 0b011: return "^*";
    case 0b100: return "v**";
    case 0b101: return "^**";
    case 0b001000: return "??TERMSRV_SET_LED down??";
    case 0b001001: return "??TERMSRV_SET_LED up??";
    case 0b010000: return "??TERMSRV_SHADOW down??";
    case 0b010001: return "??TERMSRV_SHADOW up??";
    case 0b100000: return "??TERMSRV_VKPACKET down??";
    case 0b100001: return "??TERMSRV_VKPACKET up??";
    }
    return "???" + to_string(state);
}

int obfuscateVKey(int vk)
{
    return vk ^ 0b0101010101010101;
}
int deObfuscateVKey(int vk)
{
    return vk ^ 0b0101010101010101;
}
