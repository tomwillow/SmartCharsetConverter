/*
 * Reference:
 *
 * https://vietunicode.sourceforge.net/charset/
 */
#pragma once

#include <array>
#include <string>
#include <stdexcept>

namespace viet {

enum class Encoding { UTF8, VNI, VPS, VISCII, TCVN3 };

class ParseError : public std::runtime_error {
public:
    ParseError(std::string content, int position)
        : std::runtime_error("parse error"), content(content), position(position) {}

private:
    std::string content;
    int position;
};

void Init() noexcept;

bool CheckEncoding(const char *str, int len, Encoding encoding) noexcept;
bool CheckEncoding(const std::string &str, Encoding encoding) noexcept;

/**
 * Convert TCVN3 etc. encodings to utf8 string.
 * @exception ParseError thrown when parse fail.
 */
std::string ConvertToUtf8(const char *src, int srcSize, Encoding srcEncoding);

/**
 * Convert to TCVN3 etc. encodings from utf8 string.
 * @exception ParseError thrown when parse fail.
 */
std::string ConvertFromUtf8(const std::string &utf8Str, Encoding destEncoding);

} // namespace viet