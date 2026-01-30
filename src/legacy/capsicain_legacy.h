#include <string>
#include <map>

#include "platform/interception.h"
#include "legacy/utils.h"
#include "legacy/configUtils.h"
#include "legacy/traybar.h"

// Debug logging - runtime conditional based on options
#define IFDEBUG if(options.debug && !globalState.secretSequenceRecording)

// Trace and profiling - compile-time disabled (can be re-enabled by changing to true)
constexpr bool ENABLE_TRACE = false;
constexpr bool ENABLE_PROFILING = false;

struct Executable
{
    std::string verb;
    std::string path;
    std::string args;
    std::string dir;
    int mode;
    HANDLE proc;
    DWORD pid;
    HWND hwnd;
};

struct Device {
    std::string id;
    bool keyboard;
    bool apple;
};

enum KEYSTATE
{
    KEYSTATE_DOWN = 0,
    KEYSTATE_UP = 1,
    KEYSTATE_E0_DOWN = 2,
    KEYSTATE_E0_UP = 3,
    KEYSTATE_E1_DOWN = 4,
    KEYSTATE_E1_UP = 5
};

void keySequenceAppendMakeKey(unsigned short scancode, std::vector<VKeyEvent> &sequence);
void keySequenceAppendBreakKey(unsigned short scancode, std::vector<VKeyEvent> &sequence);
void keySequenceAppendMakeBreakKey(unsigned short scancode, std::vector<VKeyEvent> &sequence);

std::string getSymbolForIKStrokeState(unsigned short state);

bool processOnOffKey();
void InterceptionSendCurrentKeystroke();
void handleConfigSwitch(int scancode);
// processCommand() replaced by CommandHandler class (see commands/CommandHandler.h)
void processModifierState();
bool processMessyKeys();
void processRewireScancodeToVirtualcode();
void processCombos();
void processMapAlphaKeys();

void detectTapping();
void playKeyEventSequence(std::vector<VKeyEvent> keyEventSequence);

void printOptions();

void sendVKeyEvent(VKeyEvent keyEvent, bool hold = true);

void sendResultingKeyOrSequence();

VKeyEvent convertIkstroke2VKeyEvent(InterceptionKeyStroke ikStroke);

void normalizeIKStroke(InterceptionKeyStroke &ikstroke);
InterceptionKeyStroke convertVkeyEvent2ikstroke(VKeyEvent keyEvent);
std::map<uint8_t, Device>* getHardwareId(bool refresh = true);

bool initConsoleWindow();

// Forward declaration for ConfigurationService
namespace capsicain { namespace services { class ConfigurationService; } }
void parseIniGlobals(capsicain::services::ConfigurationService& configService);

void printHelloHeader();
void printStatus();
void printKeylabels();
void printHelp();
void printIKStrokeState(InterceptionKeyStroke iks);
void printLoopState1Input();
void printLoopState2Modifier();
void printLoopStateMappingTime(long us);
void printLoopState4TapState();


void reset();
void reload(capsicain::services::ConfigurationService& configService);
void releaseAllSentKeys();
std::vector<std::string> assembleConfig(int config);
void switchConfig(int config, bool forceReloadSameLayer);
void resetCapsNumScrollLock();

int obfuscateVKey(int vk);
int deObfuscateVKey(int vk);

int getKeyHolding(int vcode);
bool runExecutable(Executable &exe);
void killExecutable(Executable &exe);
void killExecutableByPath(std::string path);

void loadAHK();
void unloadAHK();
void sendAHK(const std::string& msg);
