/*
 * Reference:
 *
 * https://vietunicode.sourceforge.net/charset/
 */
#pragma once

#include <array>
#include <string>

namespace viet {

enum class Encoding { UTF8, VNI, VPS, VISCII, TCVN3 };

struct Rune {
    std::string utf8;
    std::string vni;
    char vps;
    char viscii;
    std::string tcvn3;
    std::string description;
};

std::string ConvertToUtf8(const std::string &data, Encoding srcEncoding);

std::string ConvertFromUtf8(const std::string &data, Encoding destEncoding);

} // namespace viet