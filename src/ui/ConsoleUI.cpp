#include "platform/pch.h"
#include <Windows.h>
#include "ConsoleUI.h"

#include <iostream>
#include <iomanip>
#include "legacy/capsicain_legacy.h"
#include "platform/constants.h"
#include "legacy/utils.h"
#include "legacy/modifiers.h"
#include "platform/interception.h"

using namespace std;

// External global state (accessed by print functions)
extern struct Options {
    bool debug;
    int delayForKeySequenceMS;
    bool flipZy;
    bool flipAltWinOnAppleKeyboards;
    bool LControlLWinBlocksAlphaMapping;
    bool processOnlyFirstKeyboard;
} options;

extern struct Globals {
    std::string iniVersion;
    int capsicainOnOffKey;
} globals;

extern struct GlobalState {
    int activeConfig;
    std::string activeConfigName;
    int deviceIdKeyboard;
    bool deviceIsAppleKeyboard;
    bool keysDownSent[255];
    int secretSequenceRecording;
} globalState;

extern struct InterceptionState {
    int interceptionDevice;
    InterceptionKeyStroke currentIKstroke;
} interceptionState;

extern struct LoopState {
    int vcode;
    int scancode;
    bool tapped;
    bool tappedSlow;
    bool tapHoldMake;
} loopState;

extern struct ModifierState {
    MOD modifierDown;
    MOD modifierTapped;
    int activeDeadkey;
    int tapAndHoldKey;
} modifierState;

extern struct AllMaps {
    std::map<uint8_t, Device> devices;
} allMaps;

extern struct ProfilingTimer {
    int countIncoming;
    int countOutgoing;
    unsigned long totalMappingTimeUS;
    unsigned long totalSendingTimeUS;
    unsigned long worstMappingTimeUS;
    unsigned long worstSendingTimeUS;
} profiler;

extern std::string PRETTY_VK_LABELS[];
extern std::string errorLog;

// External functions
extern std::string getPrettyVKLabel(int vcode);
extern std::string getSymbolForIKStrokeState(unsigned short state);

ConsoleUI::ConsoleUI()
{
    // Constructor - no initialization needed currently
}

void ConsoleUI::printHeader()
{
    string line1 = "Capsicain v" VERSION;
#ifdef NDEBUG
    line1 += " (Release build)";
#else
    line1 += " (DEBUG build)";
#endif
    size_t linelen = line1.length();

    cout << endl;
    for (size_t i = 0; i < linelen; i++)
        cout << "-";
    cout << endl << line1 << endl;
    for (size_t i = 0; i < linelen; i++)
        cout << "-";
    cout << endl;
}

void ConsoleUI::printOptions()
{
    cout
        << endl << endl << "OPTIONs"
        << endl << (options.debug ? "ON :" : "off: --") << " debug output for each key event"
        << endl << (options.flipZy ? "ON :" : "off: --") << " Z <-> Y"
        << endl << (options.flipAltWinOnAppleKeyboards ? "ON :" : "off: --") << " Alt <-> Win for Apple keyboards"
        << endl << (options.LControlLWinBlocksAlphaMapping ? "ON :" : "off: --") << " Left Control and Win block alpha key mapping ('Ctrl + C is never changed')"
        << endl << (options.processOnlyFirstKeyboard ? "ON :" : "off: --") << " Process only the keyboard that sent the first key"
        << endl
        ;
}

void ConsoleUI::printStatus()
{
    int numMakeSent = 0;
    for (int i = 0; i < 255; i++)
    {
        if (globalState.keysDownSent[i])
            numMakeSent++;
    }
    cout << "STATUS" << endl << endl
        << "Capsicain version: " << VERSION << endl
        << "ini version: " << globals.iniVersion << endl
        << "active config: " << globalState.activeConfig << " = " << globalState.activeConfigName << endl
        << "Capsicain on/off key: [" << (globals.capsicainOnOffKey >= 0 ? getPrettyVKLabel(globals.capsicainOnOffKey) : "(not defined)") << "]" << endl
        << "keyboard device id: " << globalState.deviceIdKeyboard << endl
        << "Apple keyboard: " << globalState.deviceIsAppleKeyboard << endl
        << "delay between keys in sequences (ms): " << options.delayForKeySequenceMS << endl
        << "number of keys-down sent: " << dec << numMakeSent << endl
        << (errorLog.length() > 1 ? "ERROR LOG contains entries" : "clean error log") << " (" << dec << errorLog.length() << " chars)"
        ;

    IFPROF cout << endl << endl << "Profiling statistics (microseconds)"
        << endl << "Incoming / Sent out: " << profiler.countIncoming << " / " << profiler.countOutgoing
        << endl << "Average mapping time: " << profiler.totalMappingTimeUS / profiler.countOutgoing
        << endl << "Average sending time: " << profiler.totalSendingTimeUS / profiler.countOutgoing
        << endl << "Worst mapping time: " << profiler.worstMappingTimeUS
        << endl << "Worst sending time: " << profiler.worstSendingTimeUS
        ;

    IFDEBUG {
        cout << endl << endl << "Interception keyboards:";
        for (int i = 1; i <= INTERCEPTION_MAX_DEVICE; ++i)
        {
            if (allMaps.devices.find(i) != allMaps.devices.end())
            {
                if (i == 11)
                    cout << endl << "Interception mice:";
                cout << endl << i << ": " << allMaps.devices[i].id;
            }
        }
    }

    printOptions();
}

void ConsoleUI::printIKStrokeState(InterceptionKeyStroke iks)
{
    cout << endl << "IKS: " << hex << iks.code
        << " " << iks.state
        << " = " << getPrettyVKLabel(iks.code)
        << " i" << iks.information;
}

void ConsoleUI::printLoopState1Input()
{
    cout
        << " ["
        << dec << setw(2) << interceptionState.interceptionDevice << " " << hex << setw(2) << interceptionState.currentIKstroke.code << " " << interceptionState.currentIKstroke.state
        << "= " << setw(8) << (loopState.vcode == loopState.scancode ? "" : PRETTY_VK_LABELS[loopState.scancode] + " > ")
        << setw(8) << getPrettyVKLabel(loopState.vcode) << setw(2) << left << getSymbolForIKStrokeState(interceptionState.currentIKstroke.state) << right
        << "] ";
}

void ConsoleUI::printLoopState2Modifier()
{
    string mdown = modifierState.modifierDown > 0 ? stringIntToHex(modifierState.modifierDown, 0) : "";
    string mtapp = modifierState.modifierTapped > 0 ? stringIntToHex(modifierState.modifierTapped, 0) : "";
    cout << "[M:" << setw(8) << mdown
        << " T:" << setw(8) << mtapp
        << " D:" << setw(6) << (modifierState.activeDeadkey > 0 ? getPrettyVKLabel(modifierState.activeDeadkey) : "")
        << "] ";
}

void ConsoleUI::printLoopStateMappingTime(long us)
{
    cout << "  (" << setw(5) << dec << us << " u)";
}

void ConsoleUI::printLoopState4TapState()
{
    cout << (loopState.tappedSlow ? " (tap slow)" : "");
    cout << (loopState.tapped ? " (tap)" : "");

    IFTRACE if (loopState.tapHoldMake)
        cout << " (TapHold:" << hex << interceptionState.currentIKstroke.code << ")";
    if (modifierState.tapAndHoldKey >= 0)
        cout << " (TapHoldKey: " << hex << modifierState.tapAndHoldKey << ")";
}

void ConsoleUI::printKeylabels()
{
    for (int i = 0; i <= 255; i++)
        cout << "sc " << uppercase << hex << i << " = " << PRETTY_VK_LABELS[i] << endl;
}

void ConsoleUI::printHelp()
{
    cout << "HELP" << endl << endl
        << "Press [ESC] + [key] for core commands" << endl << endl
        << "[H] Help" << endl
        << "[X] Exit" << endl
        << "[0]..[9] switch configs. [0] is the unchangeable empty 'do nothing but listen for commands' config" << endl
        << "[W] flip ALT <-> WIN on Apple keyboards" << endl
        << "[Z] (labeled [Y] on GER keyboard): flip Y <-> Z keys" << endl
        << "[S] Status" << endl
        << "[D] Debug mode output" << endl
        << "[E] Error log" << endl
        << "[C] Print list of key labels for all scancodes" << endl
        << "[R] Reset and reload the .ini file" << endl
        << "[T] Move Taskbar icon to Tray and back" << endl
        << "[I] Show processed Ini for the active config" << endl
        << "[A] Autohotkey start" << endl
        << "[Y] autohotkeY stop" << endl
        << "[J][K][L][;] Macro Recording: Start,Stop,Playback,Copy macro definition to clipboard." << endl
        << "[,] and [.]: delay between keys in sequences -/+ 1ms " << endl
        /*<< "[Q] (dev feature) Stop the debug build if both release and debug are running" << endl*/
        << "[M] Toggle mouse input support" << endl
        << endl << "These commands work anywhere, Capsicain does not have to be the active window."
        ;
}
