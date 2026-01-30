/**
 * @file test_utils.cpp
 * @brief Unit tests for utils.cpp functions
 * 
 * Tests pure string manipulation functions and other utility functions
 * that don't require Windows-specific setup.
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

// Forward declarations of functions we're testing
bool stringStartsWith(std::string haystack, std::string needle);
std::string stringToLower(std::string str);
std::string stringToUpper(std::string str);
std::vector<std::string> stringSplit(const std::string &line, char delimiter);
bool stringToInt(std::string strval, int& result);
std::string stringCutFirstToken(std::string& line);
std::string stringCopyFirstToken(std::string line);
std::string stringGetLastToken(std::string line);
std::string stringGetRestBehindFirstToken(std::string line);
bool stringReplace(std::string& haystack, const std::string& needle, const std::string& newneedle);
std::string stringIntToHex(const unsigned int i, unsigned int minLength);

// Inline functions from utils.h
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

//==============================================================================
// stringStartsWith tests
//==============================================================================

TEST(StringUtils, StartsWith_BasicMatch) {
    EXPECT_TRUE(stringStartsWith("hello world", "hello"));
    EXPECT_TRUE(stringStartsWith("capsicain", "caps"));
    EXPECT_TRUE(stringStartsWith("test", "test"));
}

TEST(StringUtils, StartsWith_NoMatch) {
    EXPECT_FALSE(stringStartsWith("hello world", "world"));
    EXPECT_FALSE(stringStartsWith("capsicain", "sicain"));
    EXPECT_FALSE(stringStartsWith("test", "TEST")); // Case sensitive
}

TEST(StringUtils, StartsWith_EmptyStrings) {
    EXPECT_TRUE(stringStartsWith("", ""));
    EXPECT_TRUE(stringStartsWith("hello", ""));
    EXPECT_FALSE(stringStartsWith("", "hello"));
}

TEST(StringUtils, StartsWith_NeedleLongerThanHaystack) {
    EXPECT_FALSE(stringStartsWith("hi", "hello"));
    EXPECT_FALSE(stringStartsWith("a", "abc"));
}

//==============================================================================
// stringToLower tests
//==============================================================================

TEST(StringUtils, ToLower_AllCaps) {
    EXPECT_EQ(stringToLower("HELLO"), "hello");
    EXPECT_EQ(stringToLower("CAPSICAIN"), "capsicain");
}

TEST(StringUtils, ToLower_MixedCase) {
    EXPECT_EQ(stringToLower("HeLLo WoRLd"), "hello world");
    EXPECT_EQ(stringToLower("CaPsIcAiN"), "capsicain");
}

TEST(StringUtils, ToLower_AlreadyLowercase) {
    EXPECT_EQ(stringToLower("hello"), "hello");
    EXPECT_EQ(stringToLower("capsicain"), "capsicain");
}

TEST(StringUtils, ToLower_WithNumbers) {
    EXPECT_EQ(stringToLower("Hello123"), "hello123");
    EXPECT_EQ(stringToLower("TEST456"), "test456");
}

TEST(StringUtils, ToLower_EmptyString) {
    EXPECT_EQ(stringToLower(""), "");
}

//==============================================================================
// stringToUpper tests
//==============================================================================

TEST(StringUtils, ToUpper_AllLowercase) {
    EXPECT_EQ(stringToUpper("hello"), "HELLO");
    EXPECT_EQ(stringToUpper("capsicain"), "CAPSICAIN");
}

TEST(StringUtils, ToUpper_MixedCase) {
    EXPECT_EQ(stringToUpper("HeLLo WoRLd"), "HELLO WORLD");
}

TEST(StringUtils, ToUpper_AlreadyUppercase) {
    EXPECT_EQ(stringToUpper("HELLO"), "HELLO");
}

TEST(StringUtils, ToUpper_EmptyString) {
    EXPECT_EQ(stringToUpper(""), "");
}

//==============================================================================
// stringSplit tests
//==============================================================================

TEST(StringUtils, Split_BasicDelimiter) {
    auto result = stringSplit("hello,world,test", ',');
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "hello");
    EXPECT_EQ(result[1], "world");
    EXPECT_EQ(result[2], "test");
}

TEST(StringUtils, Split_SpaceDelimiter) {
    auto result = stringSplit("hello world test", ' ');
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "hello");
    EXPECT_EQ(result[1], "world");
    EXPECT_EQ(result[2], "test");
}

TEST(StringUtils, Split_NoDelimiter) {
    auto result = stringSplit("hello", ',');
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "hello");
}

TEST(StringUtils, Split_EmptyString) {
    auto result = stringSplit("", ',');
    // Empty string results in empty vector (no tokens)
    ASSERT_EQ(result.size(), 0);
}

//==============================================================================
// stringToInt tests
//==============================================================================

TEST(StringUtils, ToInt_ValidPositive) {
    int result = 0;
    EXPECT_TRUE(stringToInt("123", result));
    EXPECT_EQ(result, 123);
}

TEST(StringUtils, ToInt_ValidNegative) {
    int result = 0;
    EXPECT_TRUE(stringToInt("-456", result));
    EXPECT_EQ(result, -456);
}

TEST(StringUtils, ToInt_Zero) {
    int result = -1;
    EXPECT_TRUE(stringToInt("0", result));
    EXPECT_EQ(result, 0);
}

TEST(StringUtils, ToInt_InvalidString) {
    int result = 0;
    EXPECT_FALSE(stringToInt("abc", result));
}

TEST(StringUtils, ToInt_MixedString) {
    int result = 0;
    // stoi will parse the leading number portion
    EXPECT_TRUE(stringToInt("123abc", result));
    EXPECT_EQ(result, 123);
}

//==============================================================================
// stringCutFirstToken tests
//==============================================================================

TEST(StringUtils, CutFirstToken_MultipleTokens) {
    std::string line = "first second third";
    std::string token = stringCutFirstToken(line);
    EXPECT_EQ(token, "first");
    EXPECT_EQ(line, "second third");
}

TEST(StringUtils, CutFirstToken_SingleToken) {
    std::string line = "only";
    std::string token = stringCutFirstToken(line);
    EXPECT_EQ(token, "only");
    EXPECT_EQ(line, "");
}

TEST(StringUtils, CutFirstToken_LeadingSpaces) {
    std::string line = "   first second";
    std::string token = stringCutFirstToken(line);
    EXPECT_EQ(token, "first");
    EXPECT_EQ(line, "second");
}

TEST(StringUtils, CutFirstToken_EmptyString) {
    std::string line = "";
    std::string token = stringCutFirstToken(line);
    EXPECT_EQ(token, "");
    EXPECT_EQ(line, "");
}

//==============================================================================
// stringCopyFirstToken tests
//==============================================================================

TEST(StringUtils, CopyFirstToken_MultipleTokens) {
    EXPECT_EQ(stringCopyFirstToken("first second third"), "first");
}

TEST(StringUtils, CopyFirstToken_SingleToken) {
    EXPECT_EQ(stringCopyFirstToken("only"), "only");
}

TEST(StringUtils, CopyFirstToken_LeadingSpaces) {
    EXPECT_EQ(stringCopyFirstToken("   first second"), "first");
}

TEST(StringUtils, CopyFirstToken_EmptyString) {
    EXPECT_EQ(stringCopyFirstToken(""), "");
}

//==============================================================================
// stringGetLastToken tests
//==============================================================================

TEST(StringUtils, GetLastToken_MultipleTokens) {
    EXPECT_EQ(stringGetLastToken("first second third"), "third");
}

TEST(StringUtils, GetLastToken_SingleToken) {
    EXPECT_EQ(stringGetLastToken("only"), "only");
}

TEST(StringUtils, GetLastToken_TrailingSpace) {
    EXPECT_EQ(stringGetLastToken("first second "), "");
}

//==============================================================================
// stringGetRestBehindFirstToken tests
//==============================================================================

TEST(StringUtils, GetRestBehindFirstToken_MultipleTokens) {
    EXPECT_EQ(stringGetRestBehindFirstToken("first second third"), "second third");
}

TEST(StringUtils, GetRestBehindFirstToken_SingleToken) {
    EXPECT_EQ(stringGetRestBehindFirstToken("only"), "");
}

TEST(StringUtils, GetRestBehindFirstToken_LeadingSpaces) {
    EXPECT_EQ(stringGetRestBehindFirstToken("   first second"), "second");
}

//==============================================================================
// stringReplace tests
//==============================================================================

TEST(StringUtils, Replace_BasicReplace) {
    std::string str = "hello world";
    EXPECT_TRUE(stringReplace(str, "world", "there"));
    EXPECT_EQ(str, "hello there");
}

TEST(StringUtils, Replace_NoMatch) {
    std::string str = "hello world";
    EXPECT_FALSE(stringReplace(str, "xyz", "abc"));
    EXPECT_EQ(str, "hello world");
}

TEST(StringUtils, Replace_FirstOccurrence) {
    std::string str = "hello hello";
    EXPECT_TRUE(stringReplace(str, "hello", "hi"));
    EXPECT_EQ(str, "hi hello"); // Only first occurrence
}

TEST(StringUtils, Replace_RemoveNeedle) {
    std::string str = "hello world";
    EXPECT_TRUE(stringReplace(str, " world", ""));
    EXPECT_EQ(str, "hello");
}

//==============================================================================
// stringIntToHex tests
//==============================================================================

TEST(StringUtils, IntToHex_BasicConversion) {
    EXPECT_EQ(stringIntToHex(255, 2), "ff");
    EXPECT_EQ(stringIntToHex(16, 2), "10");
}

TEST(StringUtils, IntToHex_Padding) {
    EXPECT_EQ(stringIntToHex(1, 4), "0001");
    EXPECT_EQ(stringIntToHex(255, 4), "00ff");
}

TEST(StringUtils, IntToHex_Zero) {
    EXPECT_EQ(stringIntToHex(0, 2), "00");
}

TEST(StringUtils, IntToHex_LargeNumber) {
    EXPECT_EQ(stringIntToHex(65535, 4), "ffff");
}

//==============================================================================
// ltrim and rtrim tests
//==============================================================================

TEST(StringUtils, Ltrim_LeadingSpaces) {
    std::string s = "   hello";
    ltrim(s);
    EXPECT_EQ(s, "hello");
}

TEST(StringUtils, Ltrim_NoLeadingSpaces) {
    std::string s = "hello";
    ltrim(s);
    EXPECT_EQ(s, "hello");
}

TEST(StringUtils, Ltrim_AllSpaces) {
    std::string s = "    ";
    ltrim(s);
    EXPECT_EQ(s, "");
}

TEST(StringUtils, Rtrim_TrailingSpaces) {
    std::string s = "hello   ";
    rtrim(s);
    EXPECT_EQ(s, "hello");
}

TEST(StringUtils, Rtrim_NoTrailingSpaces) {
    std::string s = "hello";
    rtrim(s);
    EXPECT_EQ(s, "hello");
}

TEST(StringUtils, Rtrim_AllSpaces) {
    std::string s = "    ";
    rtrim(s);
    EXPECT_EQ(s, "");
}
