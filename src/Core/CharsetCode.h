#pragma once

#include <tstring.h>

// standard lib
#include <unordered_set>
#include <unordered_map>

enum class CharsetCode {
    UNKNOWN,
    EMPTY,
    NOT_SUPPORTED,

    //
    UTF8,
    UTF8BOM,

    UTF16BE,
    UTF16BEBOM,
    UTF16LE,
    UTF16LEBOM,
    UTF32BE,
    UTF32BEBOM,
    UTF32LE,
    UTF32LEBOM,

    GB18030,
    BIG5,
    SHIFT_JIS,
    EUC_JP,
    WINDOWS_1252,
    ISO_8859_1,

    CHARSET_CODE_END

    // 添加字符集需要同步修改：charsetCodeMap
};

struct MyCharset {
    std::tstring viewName; // the name shown on interface
    std::string icuName;   // the name used by icu
    std::unordered_set<std::string>
        icuNames; // if icu detected these charset names, map all of them to be the main charset
};

// 字符集code到名称的映射表
const std::unordered_map<CharsetCode, MyCharset> charsetCodeMap = {
    // CharsetCode枚举值, viewName显示名称, icuName, 可能的别名
    {CharsetCode::UNKNOWN, MyCharset{TEXT("未知"), "-", {}}},
    {CharsetCode::EMPTY, MyCharset{TEXT("空"), "-", {}}},
    {CharsetCode::NOT_SUPPORTED, MyCharset{TEXT("不支持"), "-", {}}},
    {CharsetCode::UTF8, MyCharset{TEXT("UTF-8"), "UTF-8", {"ASCII", "ANSI", "UTF8"}}},
    {CharsetCode::UTF8BOM, MyCharset{TEXT("UTF-8 BOM"), "UTF-8", {}}},
    {CharsetCode::GB18030, MyCharset{TEXT("GB18030"), "GB18030", {}}},

    {CharsetCode::UTF16LE, MyCharset{TEXT("UTF-16LE"), "UTF-16LE", {}}},
    {CharsetCode::UTF16LEBOM, MyCharset{TEXT("UTF-16LE BOM"), "UTF-16LE", {}}},
    {CharsetCode::UTF16BE, MyCharset{TEXT("UTF-16BE"), "UTF-16BE", {}}},
    {CharsetCode::UTF16BEBOM, MyCharset{TEXT("UTF-16BE BOM"), "UTF-16BE", {}}},
    {CharsetCode::UTF32LE, MyCharset{TEXT("UTF-32LE"), "UTF-32LE", {}}},
    {CharsetCode::UTF32LEBOM, MyCharset{TEXT("UTF-32LE BOM"), "UTF-32LE", {}}},
    {CharsetCode::UTF32BE, MyCharset{TEXT("UTF-32BE"), "UTF-32BE", {}}},
    {CharsetCode::UTF32BEBOM, MyCharset{TEXT("UTF-32BE BOM"), "UTF-32BE", {}}},
    {CharsetCode::BIG5, MyCharset{TEXT("BIG5"), "Big5", {"Big5"}}},
    {CharsetCode::SHIFT_JIS, MyCharset{TEXT("SHIFT-JIS"), "SHIFT-JIS", {"SHIFT_JIS"}}},
    {CharsetCode::EUC_JP, MyCharset{TEXT("EUC-JP"), "EUC-JP", {"EUC-JP"}}},
    {CharsetCode::WINDOWS_1252, MyCharset{TEXT("WINDOWS-1252"), "WINDOWS-1252", {}}},
    {CharsetCode::ISO_8859_1, MyCharset{TEXT("ISO-8859-1"), "ISO-8859-1", {}}}};

std::tstring ToViewCharsetName(CharsetCode code) noexcept;

/**
 * 编码集名字转CharsetCode
 * @exception runtime_error 未识别的字符串
 */
//
CharsetCode ToCharsetCode(const std::tstring &name);

std::string ToICUCharsetName(CharsetCode code) noexcept;

// bom串
const char UTF8BOM_DATA[] = {'\xEF', '\xBB', '\xBF'};
const char UTF16LEBOM_DATA[] = {'\xFF', '\xFE'};
const char UTF16BEBOM_DATA[] = {'\xFE', '\xFF'};
const char UTF32LEBOM_DATA[] = {'\xFF', '\xFE', '\x0', '\x0'};
const char UTF32BEBOM_DATA[] = {'\xFE', '\xFF', '\x0', '\x0'};

bool HasBom(CharsetCode code);
const char *GetBomData(CharsetCode code);
int BomSize(CharsetCode code);

/**
 * @brief 返回buf的开头是否符合某种BOM，如果都不符合返回UNKNOWN
 */
CharsetCode CheckBom(char *buf, int bufSize);
