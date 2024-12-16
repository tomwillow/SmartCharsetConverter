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

TEST(Core, uchardet_sample_test) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    const std::string uchardetSampleDir = std::string(SmartCharsetConverter_TEST_DIR) + "/uchardet_test_samples";

    std::unordered_map<std::string, CharsetCode> table; // filename, expect encoding
    for (auto path : std::filesystem::recursive_directory_iterator(uchardetSampleDir)) {
        if (path.is_directory()) {
            continue;
        }
        std::wstring expectEncoding = path.path().stem().wstring();
        table[path.path().u8string()] = ToCharsetCode(expectEncoding);
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
        std::cout << "expected: " << to_utf8(ToViewCharsetName(charsetCode)) << std::endl;
        std::cout << std::endl;
        EXPECT_EQ(charsetCode, expectedEncoding);

        SetConsoleColor();
    }
}