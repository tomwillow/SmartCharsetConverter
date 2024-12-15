#include "CharsetCode.h"

// standard
#include <stdexcept>

struct MyCharset {
    std::tstring viewName; // the name shown on interface
    std::string icuName;   // the name used by icu
    std::unordered_set<std::string>
        icuNames; // if icu detected these charset names, map all of them to be the main charset
    bool isVietnameseLocalCharset;
};

// 字符集code到名称的映射表
const std::unordered_map<CharsetCode, MyCharset> charsetCodeMap = {
    // CharsetCode枚举值, viewName显示名称, icuName, 可能的别名
    {CharsetCode::UNKNOWN, MyCharset{TEXT("未知"), "-", {}, false}},
    {CharsetCode::EMPTY, MyCharset{TEXT("空"), "-", {}, false}},
    {CharsetCode::NOT_SUPPORTED, MyCharset{TEXT("不支持"), "-", {}, false}},
    {CharsetCode::UTF8, MyCharset{TEXT("UTF-8"), "UTF-8", {"ASCII", "ANSI", "UTF8"}, false}},
    {CharsetCode::UTF8BOM, MyCharset{TEXT("UTF-8 BOM"), "UTF-8", {}, false}},
    {CharsetCode::GB18030, MyCharset{TEXT("GB18030"), "GB18030", {"GB"}, false}},

    {CharsetCode::UTF16LE, MyCharset{TEXT("UTF-16LE"), "UTF-16LE", {}, false}},
    {CharsetCode::UTF16LEBOM, MyCharset{TEXT("UTF-16LE BOM"), "UTF-16LE", {}, false}},
    {CharsetCode::UTF16BE, MyCharset{TEXT("UTF-16BE"), "UTF-16BE", {}, false}},
    {CharsetCode::UTF16BEBOM, MyCharset{TEXT("UTF-16BE BOM"), "UTF-16BE", {}, false}},
    {CharsetCode::UTF32LE, MyCharset{TEXT("UTF-32LE"), "UTF-32LE", {}, false}},
    {CharsetCode::UTF32LEBOM, MyCharset{TEXT("UTF-32LE BOM"), "UTF-32LE", {}, false}},
    {CharsetCode::UTF32BE, MyCharset{TEXT("UTF-32BE"), "UTF-32BE", {}, false}},
    {CharsetCode::UTF32BEBOM, MyCharset{TEXT("UTF-32BE BOM"), "UTF-32BE", {}, false}},
    {CharsetCode::BIG5, MyCharset{TEXT("BIG5"), "Big5", {"Big5"}, false}},
    {CharsetCode::SHIFT_JIS, MyCharset{TEXT("SHIFT-JIS"), "SHIFT-JIS", {"SHIFT_JIS"}, false}},
    {CharsetCode::EUC_JP, MyCharset{TEXT("EUC-JP"), "EUC-JP", {"EUC-JP"}, false}},
    {CharsetCode::WINDOWS_1252, MyCharset{TEXT("WINDOWS-1252"), "WINDOWS-1252", {}, false}},
    {CharsetCode::WINDOWS_1258, MyCharset{TEXT("WINDOWS-1258"), "WINDOWS-1258", {}, false}},
    {CharsetCode::ISO_8859_1, MyCharset{TEXT("ISO-8859-1"), "ISO-8859-1", {}, false}},

    {CharsetCode::VNI, MyCharset{TEXT("VNI"), "", {}, true}},
    {CharsetCode::VPS, MyCharset{TEXT("VPS"), "", {}, true}},
    {CharsetCode::VISCII, MyCharset{TEXT("VISCII"), "", {}, true}},
    {CharsetCode::TCVN3, MyCharset{TEXT("TCVN3"), "", {}, true}},
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

bool IsVietnameseLocalCharset(CharsetCode code) noexcept {
    return charsetCodeMap.at(code).isVietnameseLocalCharset;
}
