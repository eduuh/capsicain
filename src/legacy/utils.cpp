#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <windows.h>
#include <tlhelp32.h>
#include <algorithm>
#include <cctype>
#include <locale>
#include "utils.h"

// Selective using declarations
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::istreambuf_iterator;
using std::setw;
using std::setfill;
using std::hex;
namespace chrono = std::chrono;

void raise_process_priority(void) noexcept
{
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
}

void copyToClipBoard(const std::string& text)
{
    const char* output = text.c_str();
    const size_t len = strlen(output) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (hMem == 0)
    {
        cout << endl << "Cannot allocate memory.";
        return;
    }
    LPTSTR lptstrCopy = (LPTSTR) GlobalLock(hMem);
    if(lptstrCopy != 0)
        memcpy(lptstrCopy, output, len);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

DWORD FindProcessId(const string& processName)
{
    // Make a mutable copy since we need to modify it
    string processNameCopy = processName;
    char *procNameChar = &processNameCopy[0u];
    char* p = strrchr(procNameChar, '\\');
    if (p)
        procNameChar = p + 1;

    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, nullptr);
    if (processesSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    Process32First(processesSnapshot, &processInfo);
    if (!_stricmp(procNameChar, processInfo.szExeFile))
    {
        CloseHandle(processesSnapshot);
        return processInfo.th32ProcessID;
    }

    while (Process32Next(processesSnapshot, &processInfo))
    {
        if (!_stricmp(procNameChar, processInfo.szExeFile))
        {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }
    }

    CloseHandle(processesSnapshot);
    return 0;
}

string startProgram(const string& processName, const string& dir)
{
    string ret = "";
    string path = dir + "\\" + processName;
    if (0xffffffff == GetFileAttributes(path.c_str()))
    {
        ret = "file/path does not exist: " + processName;
    }
    else if (FindProcessId(processName))
    {
        ret = "already running: " + processName;
    }
    else
    {
        ShellExecuteA(nullptr, "open", path.c_str(), nullptr, dir.c_str(), SW_SHOWDEFAULT);
    }
    return ret;
}

string startProgramSameFolder(const string& processName)
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
    string::size_type pos = string(buffer).find_last_of("\\/");
    string homedir = (string(buffer).substr(0, pos));
    return startProgram(processName, homedir);
}

BOOL CALLBACK TerminateAppEnum(HWND hwnd, LPARAM lParam) noexcept
{
    DWORD dwID;

    GetWindowThreadProcessId(hwnd, &dwID);

    if (dwID == (DWORD)lParam)
    {
        PostMessage(hwnd, WM_CLOSE, 0, 0);
    }

    return TRUE;
}

// Finds a process by its name ("theapp.exe")
// Sends a "Close please" msg, if it is still running it kills it.
void closeOrKillProgram(const string& processName)
{
    DWORD timeoutMS = 1000;

    DWORD pid = FindProcessId(processName);
    if (pid == 0)
    {
        cout << endl << "Cannot find/stop process: " << processName;
        return;
    }
    HANDLE   hProc = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, pid);


    // TerminateAppEnum() posts WM_CLOSE to all windows whose PID
    // matches your process's.
    EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM)pid);


    int result = 1; // 0=fail; 1=close; 2=kill
    if (WaitForSingleObject(hProc, timeoutMS) != WAIT_OBJECT_0)
        result = (TerminateProcess(hProc, 0) ? 2 : 0);
    CloseHandle(hProc);

    if (result == 0)
        cout << " Cannot stop process: " << processName;
    else if (result == 1)
        cout << " Stopped.";
    else if (result == 2)
        cout << " Killed.";
}

//time stuff
unsigned long timeSinceTimepointMS(chrono::steady_clock::time_point timepoint) noexcept
{
    return (unsigned long)chrono::duration_cast<chrono::milliseconds>(std::chrono::steady_clock::now() - timepoint).count();
}
unsigned long timeSinceTimepointUS(chrono::steady_clock::time_point timepoint) noexcept
{
    return (unsigned long)chrono::duration_cast<chrono::microseconds>(std::chrono::steady_clock::now() - timepoint).count();
}
std::chrono::steady_clock::time_point timeGetTimepointNow() noexcept
{
    return std::chrono::steady_clock::now();
}
unsigned long timeBetweenTimepointsUS(std::chrono::steady_clock::time_point timepoint1, std::chrono::steady_clock::time_point timepoint2) noexcept
{
    unsigned long dura = (unsigned long) std::chrono::duration_cast<std::chrono::microseconds>(timepoint2 - timepoint1).count();
    return dura;
}

//String stuff
bool stringStartsWith(const string& haystack, const string& needle) noexcept
{
    return (haystack.compare(0, needle.length(), needle) == 0);
}
std::string stringToLower(const std::string& str) noexcept
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}
std::string stringToUpper(const std::string& str) noexcept
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<string> stringSplit(const std::string &line, char delimiter)
{
    vector<string> res;

    string s = line;
    std::replace(s.begin(), s.end(), delimiter, ' ');
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ' '))
        res.push_back(token);

    return res;
}

bool stringToInt(const string& strval, int &result)
{
    try
    {
        result = stoi(strval);
    }
    catch (...)
    {
        cout << endl << "error: stringToInt: not a number: " << strval;
        return false;
    }

    return true;
}

//trim left then cut first token
std::string stringCutFirstToken(std::string& line)
{
    line.erase(0, line.find_first_not_of(' '));
    size_t idx = line.find_first_of(" ");
    string res = "";
    if (idx == string::npos)
    {
        res = line;
        line = "";
    }
    else
    {
        res = line.substr(0, idx);
        line = line.substr(idx);
        while (line[0] == ' ')
            line = line.substr(1);
    }
    return res;
}
//trim left, then copy first token
std::string stringCopyFirstToken(const std::string& line) noexcept
{
    std::string result = line;
    result.erase(0, result.find_first_not_of(' '));
    size_t idx = result.find_first_of(" ");
    if (idx == string::npos)
        idx = result.length();
    return result.substr(0, idx);
}
std::string stringGetLastToken(const std::string& line) noexcept
{
    return line.substr(line.find_last_of(' ') + 1);
}
std::string stringGetRestBehindFirstToken(const std::string& line) noexcept
{
    std::string result = line;
    result.erase(0, result.find_first_not_of(' '));
    size_t idx = result.find_first_of(" ");
    if (idx == string::npos)
        return("");
    result = result.substr(idx);
    result.erase(0, result.find_first_not_of(' '));
    return result;
}
bool stringReplace(std::string& haystack, const std::string& needle, const std::string& newneedle) noexcept
{
    size_t start_pos = haystack.find(needle);
    if (start_pos == std::string::npos)
        return false;
    haystack.replace(start_pos, needle.length(), newneedle);
    return true;
}

std::string stringIntToHex(const unsigned int i, unsigned int minLength) noexcept
{
    std::stringstream s;
    s << setfill('0') << setw(minLength) << std::hex << i;
    return s.str();
}

size_t GetSizeOfFile(const std::wstring& path)
{
    struct _stat fileinfo;
    _wstat(path.c_str(), &fileinfo);
    return fileinfo.st_size;
}

std::wstring LoadUtf8FileToString(const std::wstring& filename)
{
    std::wstring buffer;            // stores file contents
    FILE *f;
    _wfopen_s(&f, filename.c_str(), L"rtS, ccs=UTF-8");

    // Failed to open file
    if (f == nullptr)
    {
        // ...handle some error...
        return buffer;
    }

    size_t filesize = GetSizeOfFile(filename);

    // Read entire file contents in to memory
    if (filesize > 0)
    {
        buffer.resize(filesize);
        size_t wchars_read = fread(&(buffer.front()), sizeof(wchar_t), filesize, f);
        buffer.resize(wchars_read);
        buffer.shrink_to_fit();
    }

    fclose(f);

    return buffer;
}

std::wstring widen(const std::string& s) noexcept
{
    std::wstring temp(s.length(),L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp; 
}