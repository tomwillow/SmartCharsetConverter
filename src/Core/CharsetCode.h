#pragma once

#include <Common/tstring.h>

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
    EUC_TW,

    WINDOWS_1250,
    WINDOWS_1251,
    WINDOWS_1252,
    WINDOWS_1253,
    WINDOWS_1254,
    WINDOWS_1255,
    WINDOWS_1256,
    WINDOWS_1257,
    WINDOWS_1258, // Vietnamese
    ISO_8859_1,
    ISO_8859_2,
    ISO_8859_3,
    ISO_8859_4,
    ISO_8859_5,
    ISO_8859_6,
    ISO_8859_7,
    ISO_8859_8,
    ISO_8859_9,
    ISO_8859_10,
    ISO_8859_11,
    // ISO_8859_12,  // no this charset due to history reason
    ISO_8859_13,
    ISO_8859_14,
    ISO_8859_15,
    ISO_8859_16,
    ISO_2022_JP,
    ISO_2022_KR,

    IBM852,
    IBM855,
    IBM865,
    IBM862_LOGICAL,
    IBM862_VISUAL,
    IBM866,

    CP737,

    MAC_CENTRALEUROPE,
    MAC_CYRILLIC,

    VNI,    // Vietnamese
    VPS,    // Vietnamese
    VISCII, // Vietnamese
    TCVN3,  // Vietnamese

    GEORGIAN_ACADEMY,
    GEORGIAN_PS,
    JOHAB,
    UHC,
    KOI8_R,
    TIS_620,

    CHARSET_CODE_END

    // 添加字符集需要同步修改：charsetCodeMap
};

enum class ConvertEngine {
    ICU,
    SELF_VIETNAMESE_CONVERTER,
    NO_ENGINE,

    END,
};

std::string ToViewCharsetName(CharsetCode code) noexcept;

/**
 * 编码集名字转CharsetCode
 * @exception runtime_error 未识别的字符串
 */
//
CharsetCode ToCharsetCode(const std::string &name);

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

ConvertEngine GetConvertEngine(CharsetCode code) noexcept;
