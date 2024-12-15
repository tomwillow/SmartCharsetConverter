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
    UTF8, // this line's order should not be changed. see DialogMain.cpp line 142: for (int icode =
          // static_cast<int>(CharsetCode::UTF8), i = 0;
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
    WINDOWS_1258, // Vietnamese
    ISO_8859_1,

    VNI,    // Vietnamese
    VPS,    // Vietnamese
    VISCII, // Vietnamese
    TCVN3,  // Vietnamese

    CHARSET_CODE_END

    // 添加字符集需要同步修改：charsetCodeMap
};

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

bool IsVietnameseLocalCharset(CharsetCode code) noexcept;
