#pragma once
#include <string>
#include <chrono>
#include <vector>

void raise_process_priority(void);
void copyToClipBoard(const std::string& text);
std::string startProgram(const std::string& processname, const std::string& dir);
std::string startProgramSameFolder(const std::string& path);
void closeOrKillProgram(const std::string& processName);
DWORD FindProcessId(const std::string& processName);

unsigned long timeSinceTimepointMS(std::chrono::steady_clock::time_point timepoint);
unsigned long timeSinceTimepointUS(std::chrono::steady_clock::time_point timepoint);
std::chrono::steady_clock::time_point timeGetTimepointNow();
unsigned long timeBetweenTimepointsUS(std::chrono::steady_clock::time_point timepoint1, std::chrono::steady_clock::time_point timepoint2);

bool stringStartsWith(const std::string& haystack, const std::string& needle);
std::string stringGetLastToken(const std::string& line);
std::string stringGetRestBehindFirstToken(const std::string& line);
std::string stringCutFirstToken(std::string& line);
std::string stringCopyFirstToken(const std::string& line);
std::string stringToLower(const std::string& str);
std::string stringToUpper(const std::string& str);
std::vector<std::string> stringSplit(const std::string &line, char delimiter);
bool stringToInt(const std::string& strval, int& result);
bool stringReplace(std::string& haystack, const std::string& needle, const std::string& newneedle);
std::string stringIntToHex(const unsigned int i, unsigned int minLength);
BOOL CALLBACK TerminateAppEnum(HWND hwnd, LPARAM lParam);

inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

size_t GetSizeOfFile(const std::wstring &path);
std::wstring LoadUtf8FileToString(const std::wstring &filename);
std::wstring widen(const std::string &s);
