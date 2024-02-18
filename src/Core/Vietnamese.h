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

enum class Encoding { UTF8, VNI, VPS, VISCII, TCVN3 };

inline std::string_view to_string(Encoding encoding) noexcept {
    switch (encoding) {
    case Encoding::UTF8:
        return "UTF8";
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

class ConvertError : public std::runtime_error {
public:
    ConvertError(std::string content, int position, Encoding srcEncoding, Encoding destEncoding) noexcept;

    virtual const char *what() const noexcept override {
        return errMsg.c_str();
    }

private:
    std::string content;
    int position;
    Encoding srcEncoding;
    Encoding destEncoding;
    std::string errMsg;
};

/**
 * All FUNCTIONS BELOW SHOULD CALL THIS FIRSTLY.
 */
void Init() noexcept;

bool CheckEncoding(const char *str, int len, Encoding encoding) noexcept;
bool CheckEncoding(const std::string &str, Encoding encoding) noexcept;

/**
 * Convert TCVN3 etc. encodings to utf8 string.
 * @exception ConvertError thrown when parse fail.
 */
std::string ConvertToUtf8(const char *src, int srcSize, Encoding srcEncoding);

/**
 * Convert to TCVN3 etc. encodings from utf8 string.
 * @exception ConvertError thrown when parse fail.
 */
std::string ConvertFromUtf8(const std::string_view &utf8Str, Encoding destEncoding);

/**
 * Convert any Vietnamese encoding(include utf8) to any Vietnamese encoding.
 * @exception ConvertError thrown when parse fail.
 */
std::string Convert(const char *src, int srcSize, Encoding srcEncoding, Encoding destEncoding);

/**
 * Convert any Vietnamese encoding(include utf8) to any Vietnamese encoding.
 * @exception ConvertError thrown when parse fail.
 */
std::string Convert(std::string_view src, Encoding srcEncoding, Encoding destEncoding);

} // namespace viet