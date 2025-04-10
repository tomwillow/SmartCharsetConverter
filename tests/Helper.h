#pragma once

#include <Core/CharsetCode.h>

#include <unordered_map>
#include <string>

namespace helper {

/**
 * scan directory for get the pairs of "filename" and "expected encoding".
 * @param pattern the pattern of encoding. typical example: for the file name "[UTF-8]test.txt", the pattern should be
 * R"(\[([\S]+)\].*)"
 */
std::unordered_map<std::string, CharsetCode>
ScanDirectoryForExpectedEncodingTable(const std::string &dir, const std::string &pattern = R"(\[([\S]+)\].*)");

} // namespace helper