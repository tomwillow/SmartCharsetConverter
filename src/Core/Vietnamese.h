/*
 * Reference:
 *
 * https://vietunicode.sourceforge.net/charset/
 */
#pragma once

#include <array>
#include <string>
#include <string_view>
#include <stdexcept>
#include <cassert>

namespace viet {

namespace internal {
constexpr std::size_t TABLE_LENGTH = 134;

extern const std::array<std::string, TABLE_LENGTH> utf8Table;
extern const std::array<std::u16string, TABLE_LENGTH> utf16LETable;
extern const std::array<std::string, TABLE_LENGTH> tcvn3Table;
} // namespace internal

enum class Encoding { UTF8, UTF16LE, VNI, VPS, VISCII, TCVN3 };

inline std::string_view to_string(Encoding encoding) noexcept {
    switch (encoding) {
    case Encoding::UTF8:
        return "UTF8";
    case Encoding::UTF16LE:
        return "UTF-16LE";
    case Encoding::VNI:
        return "VNI";
    case Encoding::VPS:
        return "VPS";
    case Encoding::VISCII:
        return "VISCII";
    case Encoding::TCVN3:
        return "TCVN3";
    default:
        assert(0 && "unsupported encoding");
    }
    return "";
}

inline Encoding to_encoding(std::string_view sv) noexcept {
    if (sv == "UTF8") {
        return Encoding::UTF8;
    }
    if (sv == "UTF-16LE") {
        return Encoding::UTF8;
    }
    if (sv == "VNI") {
        return Encoding::VNI;
    }
    if (sv == "VPS") {
        return Encoding::VPS;
    }
    if (sv == "VISCII") {
        return Encoding::VISCII;
    }
    if (sv == "TCVN3") {
        return Encoding::TCVN3;
    }
    assert(0);
    return Encoding::UTF8;
}

/**
 * All FUNCTIONS BELOW SHOULD CALL THIS FIRSTLY.
 */
void Init() noexcept;

bool CheckEncoding(const char *str, std::size_t len, Encoding encoding) noexcept;
bool CheckEncoding(const std::string &str, Encoding encoding) noexcept;

/**
 * Convert TCVN3 etc. encodings to utf8 string.
 * @exception ConvertError thrown when parse fail.
 */
std::string ConvertToUtf8(std::string_view src, Encoding srcEncoding);

/**
 * Convert to TCVN3 etc. encodings from utf8 string.
 * @exception ConvertError thrown when parse fail.
 */
std::string ConvertFromUtf8(std::string_view utf8Str, Encoding destEncoding);

/**
 * Convert TCVN3 etc. encodings to UTF16LE string.
 * @exception ConvertError thrown when parse fail.
 */
std::u16string ConvertToUtf16LE(std::string_view src, Encoding srcEncoding);

/**
 * Convert to TCVN3 etc. encodings from UTF16LE string.
 * @exception ConvertError thrown when parse fail.
 */
std::string ConvertFromUtf16LE(std::u16string_view utf16Str, Encoding destEncoding);

/**
 * Convert any Vietnamese encoding(include utf8) to any Vietnamese encoding.
 * @exception ConvertError thrown when parse fail.
 */
std::string Convert(std::string_view src, Encoding srcEncoding, Encoding destEncoding);

} // namespace viet