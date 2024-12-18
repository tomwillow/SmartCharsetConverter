#include "CharsetCode.h"

// standard
#include <stdexcept>

struct MyCharset {
    std::tstring viewName; // the name shown on interface
    std::string icuName;   // the name used by icu
    std::unordered_set<std::string>
        icuNames; // if icu detected these charset names, map all of them to be the main charset
    ConvertEngine convertEngine;
};

// 字符集code到名称的映射表
const std::unordered_map<CharsetCode, MyCharset> charsetCodeMap = {
    // CharsetCode枚举值, viewName显示名称, icuName, 可能的别名
    {CharsetCode::UNKNOWN, MyCharset{TEXT("Unknown"), "-", {}, ConvertEngine::ICU}},
    {CharsetCode::EMPTY, MyCharset{TEXT("-"), "-", {}, ConvertEngine::ICU}},
    {CharsetCode::NOT_SUPPORTED, MyCharset{TEXT("Not Supported"), "-", {}, ConvertEngine::ICU}},
    {CharsetCode::UTF8, MyCharset{TEXT("UTF-8"), "UTF-8", {"ASCII", "ANSI", "UTF8"}, ConvertEngine::ICU}},
    {CharsetCode::UTF8BOM, MyCharset{TEXT("UTF-8 BOM"), "UTF-8", {}, ConvertEngine::ICU}},
    {CharsetCode::GB18030, MyCharset{TEXT("GB18030"), "GB18030", {"GB"}, ConvertEngine::ICU}},

    {CharsetCode::UTF16LE, MyCharset{TEXT("UTF-16LE"), "UTF-16LE", {}, ConvertEngine::ICU}},
    {CharsetCode::UTF16LEBOM, MyCharset{TEXT("UTF-16LE BOM"), "UTF-16LE", {"utf-16"}, ConvertEngine::ICU}},
    {CharsetCode::UTF16BE, MyCharset{TEXT("UTF-16BE"), "UTF-16BE", {}, ConvertEngine::ICU}},
    {CharsetCode::UTF16BEBOM, MyCharset{TEXT("UTF-16BE BOM"), "UTF-16BE", {}, ConvertEngine::ICU}},
    {CharsetCode::UTF32LE, MyCharset{TEXT("UTF-32LE"), "UTF-32LE", {}, ConvertEngine::ICU}},
    {CharsetCode::UTF32LEBOM, MyCharset{TEXT("UTF-32LE BOM"), "UTF-32LE", {"utf-32"}, ConvertEngine::ICU}},
    {CharsetCode::UTF32BE, MyCharset{TEXT("UTF-32BE"), "UTF-32BE", {}, ConvertEngine::ICU}},
    {CharsetCode::UTF32BEBOM, MyCharset{TEXT("UTF-32BE BOM"), "UTF-32BE", {}, ConvertEngine::ICU}},
    {CharsetCode::BIG5, MyCharset{TEXT("BIG5"), "Big5", {"Big5"}, ConvertEngine::ICU}},
    {CharsetCode::SHIFT_JIS, MyCharset{TEXT("SHIFT-JIS"), "SHIFT-JIS", {"SHIFT_JIS"}, ConvertEngine::ICU}},

    {CharsetCode::EUC_JP, MyCharset{TEXT("EUC-JP"), "EUC-JP", {"EUC-JP"}, ConvertEngine::ICU}},
    {CharsetCode::EUC_TW, MyCharset{TEXT("EUC-TW"), "EUC-TW", {"EUC-TW"}, ConvertEngine::ICU}},

    {CharsetCode::WINDOWS_1250, MyCharset{TEXT("WINDOWS-1250"), "WINDOWS-1250", {}, ConvertEngine::ICU}},
    {CharsetCode::WINDOWS_1251, MyCharset{TEXT("WINDOWS-1251"), "WINDOWS-1251", {}, ConvertEngine::ICU}},
    {CharsetCode::WINDOWS_1252, MyCharset{TEXT("WINDOWS-1252"), "WINDOWS-1252", {}, ConvertEngine::ICU}},
    {CharsetCode::WINDOWS_1253, MyCharset{TEXT("WINDOWS-1253"), "WINDOWS-1253", {}, ConvertEngine::ICU}},
    {CharsetCode::WINDOWS_1254, MyCharset{TEXT("WINDOWS-1254"), "WINDOWS-1254", {}, ConvertEngine::ICU}},
    {CharsetCode::WINDOWS_1255, MyCharset{TEXT("WINDOWS-1255"), "WINDOWS-1255", {}, ConvertEngine::ICU}},
    {CharsetCode::WINDOWS_1256, MyCharset{TEXT("WINDOWS-1256"), "WINDOWS-1256", {}, ConvertEngine::ICU}},
    {CharsetCode::WINDOWS_1257, MyCharset{TEXT("WINDOWS-1257"), "WINDOWS-1257", {}, ConvertEngine::ICU}},
    {CharsetCode::WINDOWS_1258, MyCharset{TEXT("WINDOWS-1258"), "WINDOWS-1258", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_1, MyCharset{TEXT("ISO-8859-1"), "ISO-8859-1", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_2, MyCharset{TEXT("ISO-8859-2"), "ISO-8859-2", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_3, MyCharset{TEXT("ISO-8859-3"), "ISO-8859-3", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_4, MyCharset{TEXT("ISO-8859-4"), "ISO-8859-4", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_5, MyCharset{TEXT("ISO-8859-5"), "ISO-8859-5", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_6, MyCharset{TEXT("ISO-8859-6"), "ISO-8859-6", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_7, MyCharset{TEXT("ISO-8859-7"), "ISO-8859-7", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_8, MyCharset{TEXT("ISO-8859-8"), "ISO-8859-8", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_9, MyCharset{TEXT("ISO-8859-9"), "ISO-8859-9", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_10, MyCharset{TEXT("ISO-8859-10"), "ISO-8859-10", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_11, MyCharset{TEXT("ISO-8859-11"), "ISO-8859-11", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_12, MyCharset{TEXT("ISO-8859-12"), "ISO-8859-12", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_13, MyCharset{TEXT("ISO-8859-13"), "ISO-8859-13", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_14, MyCharset{TEXT("ISO-8859-14"), "ISO-8859-14", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_15, MyCharset{TEXT("ISO-8859-15"), "ISO-8859-15", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_8859_16, MyCharset{TEXT("ISO-8859-16"), "ISO-8859-16", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_2022_JP, MyCharset{TEXT("ISO-2022-jp"), "ISO-2022-jp", {}, ConvertEngine::ICU}},
    {CharsetCode::ISO_2022_KR, MyCharset{TEXT("ISO-2022-kr"), "ISO-2022-kr", {}, ConvertEngine::ICU}},

    {CharsetCode::IBM852, MyCharset{TEXT("ibm852"), "ibm852", {}, ConvertEngine::ICU}},
    {CharsetCode::IBM855, MyCharset{TEXT("ibm855"), "ibm855", {}, ConvertEngine::ICU}},
    {CharsetCode::IBM865, MyCharset{TEXT("ibm865"), "ibm865", {}, ConvertEngine::ICU}},
    {CharsetCode::IBM862_LOGICAL, MyCharset{TEXT("ibm862.logical"), "ibm862.logical", {}, ConvertEngine::ICU}},
    {CharsetCode::IBM862_VISUAL, MyCharset{TEXT("ibm862.visual"), "ibm862.visual", {}, ConvertEngine::ICU}},
    {CharsetCode::IBM866, MyCharset{TEXT("ibm866"), "ibm866", {}, ConvertEngine::ICU}},

    {CharsetCode::CP737, MyCharset{TEXT("CP737"), "CP737", {}, ConvertEngine::ICU}},

    {CharsetCode::MAC_CENTRALEUROPE,
     MyCharset{TEXT("Central Europe(Mac)"), "mac-centraleurope", {}, ConvertEngine::ICU}},
    {CharsetCode::MAC_CYRILLIC, MyCharset{TEXT("Mac Cyrillic"), "mac-cyrillic", {}, ConvertEngine::ICU}},

    {CharsetCode::VNI, MyCharset{TEXT("VNI"), "", {}, ConvertEngine::SELF_VIETNAMESE_CONVERTER}},
    {CharsetCode::VPS, MyCharset{TEXT("VPS"), "", {}, ConvertEngine::SELF_VIETNAMESE_CONVERTER}},
    {CharsetCode::VISCII, MyCharset{TEXT("VISCII"), "", {}, ConvertEngine::SELF_VIETNAMESE_CONVERTER}},
    {CharsetCode::TCVN3, MyCharset{TEXT("TCVN3"), "", {}, ConvertEngine::SELF_VIETNAMESE_CONVERTER}},

    {CharsetCode::GEORGIAN_ACADEMY, MyCharset{TEXT("georgian-academy"), "georgian-academy", {}, ConvertEngine::ICU}},
    {CharsetCode::GEORGIAN_PS, MyCharset{TEXT("georgian-ps"), "georgian-ps", {}, ConvertEngine::ICU}},
    {CharsetCode::JOHAB, MyCharset{TEXT("JOHAB"), "JOHAB", {}, ConvertEngine::ICU}},
    {CharsetCode::UHC, MyCharset{TEXT("UHC"), "UHC", {}, ConvertEngine::ICU}},
    {CharsetCode::KOI8_R, MyCharset{TEXT("koi8-r"), "koi8-r", {}, ConvertEngine::ICU}},
    {CharsetCode::TIS_620, MyCharset{TEXT("tis-620"), "tis-620", {}, ConvertEngine::ICU}},
};

std::tstring ToViewCharsetName(CharsetCode code) noexcept {
    return charsetCodeMap.at(code).viewName;
}

CharsetCode ToCharsetCode(const std::tstring &name) {
    // 查找name是否有吻合的viewName
    auto it =
        std::find_if(charsetCodeMap.begin(), charsetCodeMap.end(), [&](const std::pair<CharsetCode, MyCharset> &pr) {
            return tolower(pr.second.viewName) == tolower(name);
        });
    if (it != charsetCodeMap.end()) {
        return it->first;
    }

    // 查找name是否有吻合的icuName
    it = std::find_if(charsetCodeMap.begin(), charsetCodeMap.end(), [&](const std::pair<CharsetCode, MyCharset> &pr) {
        return tolower(pr.second.icuName) == tolower(to_string(name));
    });
    if (it != charsetCodeMap.end()) {
        return it->first;
    }

    // 查找name是否有吻合的icuNames
    for (auto &pr : charsetCodeMap) {
        for (auto &icuName : pr.second.icuNames) {
            if (tolower(icuName) == tolower(to_string(name))) {
                return pr.first;
            }
        }
    }
    throw std::runtime_error("unsupported: " + to_utf8(name));
}

std::string ToICUCharsetName(CharsetCode code) noexcept {
    return to_string(charsetCodeMap.at(code).icuName);
}

bool HasBom(CharsetCode code) {
    switch (code) {
    case CharsetCode::UTF8BOM:
    case CharsetCode::UTF16LEBOM:
    case CharsetCode::UTF16BEBOM:
        return true;
    }
    return false;
}

const char *GetBomData(CharsetCode code) {
    switch (code) {
    case CharsetCode::UTF8BOM:
        return UTF8BOM_DATA;
    case CharsetCode::UTF16LEBOM:
        return UTF16LEBOM_DATA;
    case CharsetCode::UTF16BEBOM:
        return UTF16BEBOM_DATA;
    }
    return nullptr;
}

int BomSize(CharsetCode code) {
    switch (code) {
    case CharsetCode::UTF8BOM:
        return sizeof(UTF8BOM_DATA);
    case CharsetCode::UTF16LEBOM:
        return sizeof(UTF16LEBOM_DATA);
    case CharsetCode::UTF16BEBOM:
        return sizeof(UTF16BEBOM_DATA);
    }
    return 0;
}

CharsetCode CheckBom(char *buf, int bufSize) {
    if (bufSize >= sizeof(UTF8BOM_DATA) && memcmp(buf, UTF8BOM_DATA, sizeof(UTF8BOM_DATA)) == 0) {
        return CharsetCode::UTF8BOM;
    }
    if (bufSize >= sizeof(UTF16LEBOM_DATA) && memcmp(buf, UTF16LEBOM_DATA, sizeof(UTF16LEBOM_DATA)) == 0) {
        return CharsetCode::UTF8BOM;
    }
    if (bufSize >= sizeof(UTF16BEBOM_DATA) && memcmp(buf, UTF16BEBOM_DATA, sizeof(UTF16BEBOM_DATA)) == 0) {
        return CharsetCode::UTF8BOM;
    }
    if (bufSize >= sizeof(UTF32LEBOM_DATA) && memcmp(buf, UTF32LEBOM_DATA, sizeof(UTF32LEBOM_DATA)) == 0) {
        return CharsetCode::UTF8BOM;
    }
    if (bufSize >= sizeof(UTF32BEBOM_DATA) && memcmp(buf, UTF32BEBOM_DATA, sizeof(UTF32BEBOM_DATA)) == 0) {
        return CharsetCode::UTF8BOM;
    }
    return CharsetCode::UNKNOWN;
}

ConvertEngine GetConvertEngine(CharsetCode code) noexcept {
    return charsetCodeMap.at(code).convertEngine;
}
