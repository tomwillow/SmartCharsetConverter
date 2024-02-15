#include "tstring.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <algorithm>
#include <iostream>
#include <memory>
#include <cassert>

using namespace std;

std::wstring string_to_wstring(const std::string &str) {
    LPCSTR pszSrc = str.c_str();
    int nLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
    if (nLen == 0)
        return std::wstring(L"");

    wchar_t *pwszDst = new wchar_t[nLen];
    if (!pwszDst)
        return std::wstring(L"");

    MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, nLen);
    std::wstring wstr(pwszDst);
    delete[] pwszDst;
    pwszDst = NULL;

    return wstr;
}

void MyWideCharToMultiByte(const wchar_t *wsrc, int wsrcSize, std::unique_ptr<char[]> &dest, int &destSize,
                           UINT codePage) {
    // 取得大小
    destSize = WideCharToMultiByte(codePage, 0, wsrc, wsrcSize, NULL, 0, NULL, NULL);
    if (destSize == 0) // 大小为0或者失败，返回空串
    {
        dest.reset(nullptr);
        return;
    }

    // 分配空间
    dest.reset(new char[destSize]);

    // 进行转换
    WideCharToMultiByte(codePage, 0, wsrc, wsrcSize, dest.get(), destSize, NULL, NULL);
}

void MyMultiByteToWideChar(const char *src, int srcSize, std::unique_ptr<wchar_t[]> &dest, int &destSize,
                           UINT codePage) {
    // 取得大小
    destSize = MultiByteToWideChar(codePage, 0, src, srcSize, NULL, 0);
    if (destSize == 0) // 大小为0或者失败，返回空串
    {
        dest.reset(nullptr);
        return;
    }

    // 分配空间
    dest.reset(new wchar_t[destSize]);

    // 进行转换
    MultiByteToWideChar(codePage, 0, src, srcSize, dest.get(), destSize);
}

std::string wstring_to_string(const std::wstring &wstr) {
    // 取得大小
    int nLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (nLen == 0) // 大小为0或者失败，返回空串
        return std::string("");

    // 分配空间
    unique_ptr<char[]> buf = unique_ptr<char[]>(new char[nLen]);

    // 进行转换
    nLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, buf.get(), nLen, NULL, NULL);
    if (nLen == 0) // 大小为0或者失败，返回空串
        return std::string("");

    return std::string(buf.get());
}

std::string to_string(const std::wstring &ws) {
    return wstring_to_string(ws);
}

std::string to_string(const std::string &s) {
    return s;
}

std::wstring to_wstring(const std::string &s) {
    return string_to_wstring(s);
}

std::wstring to_wstring(const std::wstring &s) {
    return s;
}

std::string to_utf8(const std::wstring &wstr) {
    // 取得大小
    int nLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (nLen == 0) // 大小为0或者失败，返回空串
        return std::string("");

    // 分配空间
    unique_ptr<char[]> buf = unique_ptr<char[]>(new char[nLen]);

    // 进行转换
    nLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buf.get(), nLen, NULL, NULL);
    if (nLen == 0) // 大小为0或者失败，返回空串
        return std::string("");

    return std::string(buf.get());
}

std::string to_utf8(const std::string &str) {
    int nwLen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    unique_ptr<wchar_t[]> pwBuf(new wchar_t[nwLen]);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.length()), pwBuf.get(), nwLen);

    int nLen = WideCharToMultiByte(CP_UTF8, 0, pwBuf.get(), -1, NULL, NULL, NULL, NULL);
    unique_ptr<char[]> pBuf(new char[nLen]);
    WideCharToMultiByte(CP_UTF8, 0, pwBuf.get(), nwLen, pBuf.get(), nLen, NULL, NULL);

    return string(pBuf.get());
}

std::string utf8_to_string(const std::string &str) {
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t *pwBuf = new wchar_t[nwLen + 1];
    memset(pwBuf, 0, nwLen * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), pwBuf, nwLen);

    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
    char *pBuf = new char[nLen + 1];
    memset(pBuf, 0, nLen + 1);
    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string ret = pBuf;
    delete[] pBuf;
    delete[] pwBuf;

    return ret;
}

std::wstring utf8_to_wstring(const std::string &str) {
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring ret(nwLen - 1, L'\0'); // -1是为了排除掉上一步预计算时添加的尾后0的长度
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), ret.data(), nwLen);

    return ret;
}

// 未测试
void FixEndLine(std::tstring &s) {
    tstring temp;
    tstring::size_type cur = 0;
    while (1) {
        auto lf = s.find_first_of(TEXT('\n'), cur);
        if (lf == tstring::npos) {
            temp += s.substr(cur);
            break;
        } else if (lf > cur && s[lf - 1] == TEXT('\r')) {
            temp += s.substr(cur, (lf + 1) - cur);
            cur = lf + 1;
        } else {
            temp += s.substr(cur, lf - cur);
            cur = lf + 1;
        }
    }
    s = temp;
}

std::string to_hex(const char *buf, int bufSize) {
    constexpr int len = 4;
    std::string hex;
    char temp[len];
    for (int i = 0; i < bufSize; ++i) {
        _snprintf_s(temp, len, "%02X ", (unsigned char)(buf[i]));
        hex += temp;
    }
    return hex;
}

std::string to_hex(std::string s) {
    constexpr int len = 4;
    std::string hex;
    char temp[len];
    for (char c : s) {
        _snprintf_s(temp, len, "%02X ", (unsigned char)c);
        hex += temp;
    }
    return hex;
}

std::wstring to_hex(std::wstring s) {
    std::wstring hex;
    wchar_t temp[5];
    for (auto c : s) {
        _snwprintf_s(temp, 5, 5, L"%04X", c);
        hex += temp;
    }
    return hex;
}

// https://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
std::tistream &safeGetline(std::tistream &is, std::tstring &t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::tistream::sentry se(is, true);
    std::tstreambuf *sb = is.rdbuf();

    for (;;) {
        TCHAR c = sb->sbumpc();
        switch (c) {
        case TEXT('\n'):
            return is;
        case TEXT('\r'):
            if (sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case std::tstreambuf::traits_type::eof():
            // Also handle the case when the last line has no line ending
            if (t.empty())
                is.setstate(std::tios::eofbit);
            return is;
        default:
            t += c;
        }
    }
}

std::vector<std::tstring_view> Split(std::tstring_view s, const std::tstring &dep) noexcept {
    std::vector<std::tstring_view> ans;
    tstring::size_type beg = 0;
    while (1) {
        // beg:  ""->npos " a"->1 "a"->0
        beg = s.find_first_not_of(dep);
        if (beg == string::npos) {
            // s == ""
            break;
        }
        s = s.substr(beg);

        // end:  "a "->1 "a"->npos
        auto end = s.find_first_of(dep);
        if (end == string::npos) {
            ans.push_back(s);
            break;
        }
        ans.push_back(s.substr(0, end));
        s = s.substr(end);
    }
    return ans;
}

void Split_UnitTest() {
    assert(Split(TEXT(""), TEXT(" ")) == vector<tstring_view>{});
    assert((Split(TEXT("a"), TEXT(" ")) == vector<tstring_view>{TEXT("a")}));
    assert((Split(TEXT("  a  "), TEXT(" ")) == vector<tstring_view>{TEXT("a")}));
    assert((Split(TEXT("  a"), TEXT(" ")) == vector<tstring_view>{TEXT("a")}));
    assert((Split(TEXT("a  "), TEXT(" ")) == vector<tstring_view>{TEXT("a")}));
    assert((Split(TEXT("a b c"), TEXT(" ")) == vector<tstring_view>{TEXT("a"), TEXT("b"), TEXT("c")}));
    assert((Split(TEXT(" a b c"), TEXT(" ")) == vector<tstring_view>{TEXT("a"), TEXT("b"), TEXT("c")}));
    assert((Split(TEXT("a b c "), TEXT(" ")) == vector<tstring_view>{TEXT("a"), TEXT("b"), TEXT("c")}));
    assert((Split(TEXT(" a b c "), TEXT(" ")) == vector<tstring_view>{TEXT("a"), TEXT("b"), TEXT("c")}));
    assert((Split(TEXT("a\tb c\t"), TEXT(" \t")) == vector<tstring_view>{TEXT("a"), TEXT("b"), TEXT("c")}));
}