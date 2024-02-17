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

void Init() noexcept;

bool CheckEncoding(const char *str, int len, Encoding encoding) noexcept;
bool CheckEncoding(const std::string &str, Encoding encoding) noexcept;

std::string ConvertToUtf8(const std::string &data, Encoding srcEncoding);

std::string ConvertFromUtf8(const std::string &data, Encoding destEncoding);

} // namespace viet