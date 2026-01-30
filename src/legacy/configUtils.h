#pragma once
#include <string>
#include <vector>
#include "platform/constants.h"
#include "legacy/modifiers.h"

const int CPS_ESC_SEQUENCE_TYPE_TEMPALTERMODIFIERS = 1;
const int CPS_ESC_SEQUENCE_TYPE_SLEEP = 2;

struct VKeyEvent
{
    int vcode = 0;
    bool isDownstroke = true;
};

bool readSanitizeIniFile(std::vector<std::string>& iniLines);

std::vector<std::string> getSectionFromIni(const std::string& sectionName, const std::vector<std::string>& iniContent);
std::vector<std::string> getTaggedLinesFromIni(const std::string& tag, const std::vector<std::string>& iniContent);
bool configHasKey(const std::string& key, const std::vector<std::string>& iniLines);
bool configHasTaggedKey(const std::string& tag, const std::string& key, const std::vector<std::string>& sectionLines);
bool getStringValueForTaggedKey(const std::string& tag, const std::string& key, std::string& value, const std::vector<std::string>& sectionLines);
bool getStringValueForKey(const std::string& key, std::string& value, const std::vector<std::string>& sectionLines);
bool getIntValueForTaggedKey(const std::string& tag, const std::string& key, int& value, const std::vector<std::string>& sectionLines);
bool getIntValueForKey(const std::string& key, int& value, const std::vector<std::string>& sectionLines);
bool parseFunctionCombo(const std::string& funcParams, std::string * scLabels, std::vector<VKeyEvent> &strokeSeq, bool releaseTemp = false, int times = 1);
bool parseFunctionHold(const std::string& funcParams, std::string *scLabels, std::vector<VKeyEvent> &strokeSeq, bool releaseAll = false, bool holdMods = false);
bool parseKeywordCombo(std::string line, int &key, MOD(&mods)[6], DEV(&devs)[2], std::vector<VKeyEvent> & strokeSequence, std::string scLabels[], const std::string& defaultFunction);
bool parseKeywordsAlpha_FromTo(const std::string& mapFromTo, int(&alphamap)[MAX_VCODES], std::string scLabels[]);
bool parseKeywordRewire(const std::string& line, int & keyA, int & keyB, int & keyC, int & keyD, std::string scLabels[]);
bool parseComboParams(std::string funcParams, std::vector<int> &vcodes, std::string *scLabels);