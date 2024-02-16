#pragma once
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> //TEXT macro
#include <tchar.h>

#include <memory>
#include <vector>
#include <algorithm>

#ifdef _UNICODE

#define tstring wstring

#define tstring_view wstring_view

#define tios wios

#define tistream wistream

#define tostream wostream

#define tifstream wifstream

#define tofstream wofstream

#define tstringstream wstringstream

#define tistringstream wistringstream

#define tostringstream wostringstream

#define tstreambuf wstreambuf

#define tcout wcout

#define to_tstring to_wstring

#define to_tstring_from_utf8 utf8_to_wstring

#else

#define tstring string

#define tstring_view string_view

#define tios ios

#define tistream istream

#define tostream ostream

#define tifstream ifstream

#define tofstream ofstream

#define tstringstream stringstream

#define tistringstream istringstream

#define tostringstream ostringstream

#define tstreambuf streambuf

#define tcout cout

#define to_tstring to_string

#define to_tstring_from_utf8 utf8_to_string

#endif

void MyWideCharToMultiByte(const wchar_t *wsrc, int wsrcSize, std::unique_ptr<char[]> &dest, int &destSize,
                           UINT codePage = CP_ACP);
void MyMultiByteToWideChar(const char *src, int srcSize, std::unique_ptr<wchar_t[]> &dest, int &destSize,
                           UINT codePage = CP_ACP);

std::wstring string_to_wstring(const std::string &str);
std::string wstring_to_string(const std::wstring &wstr);

std::string to_string(const std::wstring &ws);
std::string to_string(const std::string &s);

std::wstring to_wstring(const std::string &s);
std::wstring to_wstring(const std::wstring &s);

template <typename T>
std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>, T> tolower(const T &s) noexcept {
    using C = T::value_type;
    T ret{s};
    std::for_each(ret.begin(), ret.end(), [](C &c) {
        c = tolower(c);
    });
    return ret;
}

std::string to_utf8(const std::wstring &wstr);
std::string to_utf8(const std::string &str);

std::string utf8_to_string(const std::string &str);
std::wstring utf8_to_wstring(const std::string &str);

std::string to_hex(const char *buf, int bufSize);
std::string to_hex(std::string s);
std::wstring to_hex(std::wstring s);

std::tistream &safeGetline(std::tistream &is, std::tstring &t);

/**
 * 切分字符串。
 * dep填入分隔符，可以支持多种分隔符。例如"\n\t"。
 */
std::vector<std::tstring_view> Split(std::tstring_view s, const std::tstring &dep) noexcept;

template <typename... Args>
std::string MyPrintf(const std::string &fmtStr, std::size_t reservedBytes, Args... args) {
    std::string dest(fmtStr.size() + reservedBytes, '\0');
    snprintf(dest.data(), dest.size(), fmtStr.c_str(), args...);
    dest.erase(dest.begin() + dest.find_last_not_of('\0') + 1, dest.end());
    return dest;
}