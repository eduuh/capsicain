#include "services/AutoHotkeyService.h"
#include "legacy/utils.h"
#include <iostream>
#include <vector>

namespace capsicain {
namespace services {

bool AutoHotkeyService::initializeDLL()
{
    if (dllHandle_) {
        return true;  // Already loaded
    }

    // Try loading AutoHotkey64.dll first, then AutoHotkey.dll
    dllHandle_ = LoadLibrary(TEXT("AutoHotkey64.dll"));
    if (!dllHandle_) {
        dllHandle_ = LoadLibrary(TEXT("AutoHotkey.dll"));
    }

    if (!dllHandle_) {
        std::cout << std::endl
            << "AHK: No AutoHotkey64.dll found. Get one from "
            << "https://github.com/thqby/AutoHotkey_H";
        return false;
    }

    return loadFunctionPointers();
}

bool AutoHotkeyService::loadFunctionPointers()
{
    if (!dllHandle_) {
        return false;
    }

    threadFunc_ = (AHKTHREAD)GetProcAddress(dllHandle_, "NewThread");
    readyFunc_ = (AHKREADY)GetProcAddress(dllHandle_, "ahkReady");
    addScriptFunc_ = (AHKADDSCRIPT)GetProcAddress(dllHandle_, "addScript");
    execFunc_ = (AHKEXEC)GetProcAddress(dllHandle_, "ahkExec");
    findFuncFunc_ = (AHKFINDFUNC)GetProcAddress(dllHandle_, "ahkFindFunc");
    functionFunc_ = (AHKFUNCTION)GetProcAddress(dllHandle_, "ahkFunction");
    postFunctionFunc_ = (AHKFUNCTION)GetProcAddress(dllHandle_, "ahkPostFunction");

    return threadFunc_ && postFunctionFunc_;  // At minimum we need these
}

bool AutoHotkeyService::initialize(const std::wstring& script)
{
    if (!initializeDLL()) {
        return false;
    }

    if (script.empty()) {
        return false;
    }

    // Check for Persistent or hotkeys
    if (script.find(L"ersistent") == std::wstring::npos &&
        script.find(L"::") == std::wstring::npos) {
        std::cout << std::endl
            << "AHK: You should add \"Persistent\" to your AHK script if it doesn't have hotkeys...";
    }

    // Stop existing thread if any
    if (threadId_ && execFunc_) {
        execFunc_(L"ExitApp", threadId_);
    }

    // Start new thread
    threadId_ = threadFunc_(script.c_str(), L"", L"");
    if (threadId_) {
        std::cout << std::endl << "AHK: Loaded script to AutoHotkey64.dll";
        return true;
    } else {
        std::cout << std::endl << "AHK: Failed to load script to AutoHotkey64.dll";
        return false;
    }
}

bool AutoHotkeyService::loadScriptFromIni()
{
    if (!initializeDLL()) {
        return false;
    }

    // Load and parse [ahk] section from INI
    auto script = LoadUtf8FileToString(L"capsicain.ini");
    auto idx = script.find(L"[ahk]");
    if (idx == std::wstring::npos) {
        idx = script.find(L"[AHK]");
    }

    if (idx == std::wstring::npos) {
        std::cout << std::endl << "AHK: INI has no [ahk] section...";
        shutdown();
        return false;
    }

    script = script.substr(idx + 6);  // Skip "[ahk]\n"
    if (script.empty()) {
        return false;
    }

    return initialize(script);
}

void AutoHotkeyService::shutdown()
{
    if (threadId_ && execFunc_) {
        execFunc_(L"ExitApp", threadId_);
    }
    if (dllHandle_) {
        FreeLibrary(dllHandle_);
    }
    dllHandle_ = nullptr;
    threadId_ = 0;
}

void AutoHotkeyService::send(const std::string& msg)
{
    if (!dllHandle_) {
        std::cerr << "AHK dll is not loaded" << std::endl;
        return;
    }
    if (!threadId_) {
        std::cerr << "AHK thread not started" << std::endl;
        return;
    }
    if (!postFunctionFunc_) {
        std::cerr << "AHK postFunction not available" << std::endl;
        return;
    }

    // Split message by commas (function name and up to 10 args)
    auto args = stringSplit(msg, ',');
    std::vector<std::wstring> wargs;
    for (int i = 0; i <= 10; ++i) {
        if (i < static_cast<int>(args.size())) {
            wargs.push_back(widen(args[i]));
        } else {
            wargs.push_back(L"");
        }
    }

    // Call AHK postFunction with all 10 parameters
    postFunctionFunc_(
        wargs[0].c_str(), wargs[1].c_str(), wargs[2].c_str(),
        wargs[3].c_str(), wargs[4].c_str(), wargs[5].c_str(),
        wargs[6].c_str(), wargs[7].c_str(), wargs[8].c_str(),
        wargs[9].c_str(), wargs[10].c_str(), threadId_
    );
}

} // namespace services
} // namespace capsicain
