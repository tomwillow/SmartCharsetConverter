#include "config.h"

#include "memory_leak_detection.h"
#include "Helper.h"

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
        ASSERT_EQ(err.GetUnassignedChar(), std::string(u8"小舟从此逝，江海寄余生。"));
    }
}

TEST(Core, DetectEncodingMulti) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    auto table =
        helper::ScanDirectoryForExpectedEncodingTable(std::string(SmartCharsetConverter_TEST_DIR) + "/expect_pass");

    CoreInitOption opt;
    Core core("temp.json", opt);

    for (auto [filename, expectedEncoding] : table) {
        auto [buf, len] = ReadFileToBuffer(filename);
        auto charsetCode = DetectEncoding(core.GetUCharDet().get(), buf.get(), len);

        if (charsetCode == expectedEncoding) {
            continue;
        }
        std::cout << std::string(20, '=') << std::endl;
        std::cout << "file: " << filename << std::endl;
        std::cout << "detect: " << ToViewCharsetName(charsetCode) << std::endl;
        std::cout << "expected: " << ToViewCharsetName(expectedEncoding) << std::endl;
        std::cout << std::endl;
        EXPECT_EQ(charsetCode, expectedEncoding);

        SetConsoleColor();
    }
}
