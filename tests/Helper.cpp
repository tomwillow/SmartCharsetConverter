#include "Helper.h"

#include <Core/CharsetCode.h>

#include <fmt/format.h>

#include <filesystem>
#include <regex>

namespace helper {

/**
 * scan directory for get the pairs of "filename" and "expected encoding".
 * @param pattern the pattern to tell regular expression engine how to get the encoding name from stem of file name.
 *          for example: for the file name "[UTF-8]test.txt", its stem is "[UTF-8]test", the pattern could be
 *          R"(\[([\S]+)\].*)" to catch the encoding part "UTF-8".
 */
std::unordered_map<std::string, CharsetCode> ScanDirectoryForExpectedEncodingTable(const std::string &dir,
                                                                                   const std::string &pattern) {

    std::unordered_map<std::string, CharsetCode> table; // filename, expect encoding
    std::regex r(pattern);
    for (auto path : std::filesystem::recursive_directory_iterator(std::filesystem::u8path(dir))) {
        if (path.is_directory()) {
            continue;
        }

        std::string stem = path.path().stem().u8string();
        std::smatch ret;
        bool ok = std::regex_match(stem, ret, r);
        if (!ok) {
            throw std::runtime_error(
                fmt::format("encoding description not found in filename: {}\nbasename should match this pattern {}",
                            path.path().string(), pattern));
        }
        table[path.path().u8string()] = ToCharsetCode(ret[1]);
    }
    return table;
}

} // namespace helper