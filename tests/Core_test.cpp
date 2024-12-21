#include "config.h"

#include "memory_leak_detection.h"

#include <Core/Language.h>
#include <Core/Core.h>
#include <Core/Detect.h>
#include <Common/FileFunction.h>
#include <Common/ConsoleSettings.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <unordered_map>
#include <regex>

TEST(Core, EncodeWithUnassignedChars) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8
    // MemoryLeakDetection mld;

    try {
        Encode(u"abcdefg小舟从此逝，江海寄余生。asdfghjkl", CharsetCode::WINDOWS_1252);
        FAIL();
    } catch (const UnassignedCharError &err) {
        ASSERT_EQ(std::string(err.what()), std::string(u8"小舟从此逝，江海寄余生。"));
    }
}

TEST(Core, DetectEncodingMulti) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    const std::string uchardetSampleDir = std::string(SmartCharsetConverter_TEST_DIR) + "/expect_pass";

    std::unordered_map<std::string, CharsetCode> table; // filename, expect encoding
    for (auto path : std::filesystem::recursive_directory_iterator(uchardetSampleDir)) {
        if (path.is_directory()) {
            continue;
        }

        std::string stem = path.path().stem().u8string();
        std::regex r(R"(\[([\S]+)\].*)");
        std::smatch ret;
        bool ok = std::regex_match(stem, ret, r);
        if (!ok) {
            throw std::runtime_error("encoding description not found in filename: " + path.path().string() +
                                     "\nfor example: [encoding]file_original_name.txt");
        }
        table[path.path().u8string()] = ToCharsetCode(utf8_to_wstring(ret[1]));
    }

    CoreInitOption opt;
    Core core(L"temp.json", opt);

    for (auto [filename, expectedEncoding] : table) {
        auto [buf, len] = ReadFileToBuffer(utf8_to_wstring(filename));
        auto charsetCode = DetectEncoding(core.GetUCharDet().get(), buf.get(), len);

        if (charsetCode == expectedEncoding) {
            SetConsoleColor(ConsoleColor::GREEN);
        } else {
            SetConsoleColor(ConsoleColor::RED);
        }
        std::cout << std::string(20, '=') << std::endl;
        std::cout << "file: " << filename << std::endl;
        std::cout << "detect: " << to_utf8(ToViewCharsetName(charsetCode)) << std::endl;
        std::cout << "expected: " << to_utf8(ToViewCharsetName(expectedEncoding)) << std::endl;
        std::cout << std::endl;
        EXPECT_EQ(charsetCode, expectedEncoding);

        SetConsoleColor();
    }
}
