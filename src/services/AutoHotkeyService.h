#pragma once

#include <Windows.h>
#include <string>

namespace capsicain {
namespace services {

/**
 * @brief AutoHotkeyService encapsulates AutoHotkey DLL integration
 *
 * Responsibilities:
 * - Load AutoHotkey64.dll or AutoHotkey.dll
 * - Initialize AHK thread with script from INI
 * - Send messages/function calls to AHK
 * - Manage AHK lifecycle (load/unload)
 *
 * Usage:
 *   AutoHotkeyService ahk;
 *   if (ahk.initialize("script content")) {
 *       ahk.send("FunctionName,arg1,arg2");
 *   }
 *   ahk.shutdown();
 */
class AutoHotkeyService {
public:
    AutoHotkeyService() = default;
    ~AutoHotkeyService() { shutdown(); }

    // Prevent copying
    AutoHotkeyService(const AutoHotkeyService&) = delete;
    AutoHotkeyService& operator=(const AutoHotkeyService&) = delete;

    /**
     * @brief Initialize AutoHotkey DLL and load script
     * @param script Script content to load
     * @return true if successful
     */
    bool initialize(const std::wstring& script);

    /**
     * @brief Initialize without script (just load DLL)
     * @return true if DLL loaded successfully
     */
    bool initializeDLL();

    /**
     * @brief Load script from INI file [ahk] section
     * @return true if successful
     */
    bool loadScriptFromIni();

    /**
     * @brief Shutdown AHK thread and unload DLL
     */
    void shutdown();

    /**
     * @brief Send message to AHK (calls postFunction with comma-separated args)
     * @param msg Message string (e.g., "FunctionName,arg1,arg2")
     */
    void send(const std::string& msg);

    /**
     * @brief Check if AHK is loaded and ready
     */
    [[nodiscard]] bool isLoaded() const noexcept {
        return dllHandle_ != nullptr && threadId_ != 0;
    }

    /**
     * @brief Check if DLL is loaded (but thread may not be started)
     */
    [[nodiscard]] bool isDLLLoaded() const noexcept {
        return dllHandle_ != nullptr;
    }

    /**
     * @brief Get thread ID
     */
    [[nodiscard]] int getThreadId() const noexcept {
        return threadId_;
    }

private:
    // AHK function pointer types
    typedef int (*AHKTHREAD)(const wchar_t* aScript, const wchar_t* aCmdLine, const wchar_t* aTitle);
    typedef int (*AHKREADY)(int threadid);
    typedef int (*AHKADDSCRIPT)(const wchar_t* script, int waitexecute, int threadid);
    typedef int (*AHKEXEC)(const wchar_t*, int threadid);
    typedef const wchar_t* (*AHKFINDFUNC)(const wchar_t*, int threadid);
    typedef const wchar_t* (*AHKFUNCTION)(const wchar_t* func,
        const wchar_t* param1, const wchar_t* param2, const wchar_t* param3,
        const wchar_t* param4, const wchar_t* param5, const wchar_t* param6,
        const wchar_t* param7, const wchar_t* param8, const wchar_t* param9,
        const wchar_t* param10, int threadid);

    // DLL and function pointers
    HMODULE dllHandle_ = nullptr;
    int threadId_ = 0;
    AHKTHREAD threadFunc_ = nullptr;
    AHKREADY readyFunc_ = nullptr;
    AHKADDSCRIPT addScriptFunc_ = nullptr;
    AHKEXEC execFunc_ = nullptr;
    AHKFINDFUNC findFuncFunc_ = nullptr;
    AHKFUNCTION functionFunc_ = nullptr;
    AHKFUNCTION postFunctionFunc_ = nullptr;

    // Helper to load function pointers
    bool loadFunctionPointers();
};

} // namespace services
} // namespace capsicain
