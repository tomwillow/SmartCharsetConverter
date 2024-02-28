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

void fun() {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    std::string filename = std::string(SmartCharsetConverter_TEST_DIR) + "/expected.txt";
    auto [buf, len] = ReadFileToBuffer(utf8_to_wstring(filename));

    CoreInitOption opt;
    Core core(L"temp.json", opt);
    core.SetOutputTarget(Configuration::OutputTarget::TO_DIR);
    core.SetOutputDir(".");

    CharsetCode code = DetectEncoding(core.GetUCharDet().get(), buf.get(), len);
    ASSERT_EQ(code, CharsetCode::UTF8);

    std::u16string utf16leStr = Decode(std::string_view(buf.get(), len), code);
    auto lineBreak = GetLineBreaks(utf16leStr.data(), utf16leStr.size());

    core.SetOutputCharset(CharsetCode::GB18030);
    Core::ConvertFileResult ret = core.Convert(utf8_to_wstring(filename), code, lineBreak);
    ASSERT_FALSE(ret.errInfo.has_value());

    std::filesystem::rename("./expected.txt", "expected-out.txt");

    {
        core.SetOutputCharset(CharsetCode::UTF8);
        Core::ConvertFileResult ret =
            core.Convert(utf8_to_wstring(u8"./expected-out.txt"), CharsetCode::GB18030, lineBreak);
        ASSERT_FALSE(ret.errInfo.has_value());

        auto [bufOut, bufOutLen] = ReadFileToBuffer(utf8_to_wstring(u8"./expected-out.txt"));
        ASSERT_EQ(len, bufOutLen);
        ASSERT_TRUE(memcmp(buf.get(), bufOut.get(), len) == 0);
    }
}

TEST(Core, EncodeWithUnassignedChars) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8
    // MemoryLeakDetection mld;

    //
    LanguageServiceOption option;
    option.fnGetLanguageFromConfig = []() -> std::string {
        return "English";
    };
    option.resourceIds = {};
    option.resourceType = L"LanguageJson";
    InitLanguageService(option);

    try {
        Encode(u"abcdefg小舟从此逝，江海寄余生。asdfghjkl", CharsetCode::WINDOWS_1252);
        FAIL();
    } catch (const std::runtime_error &err) {
        ASSERT_EQ(
            std::string(err.what()),
            std::string(
                u8"Some characters will be lost when converting to the target encoding:小舟从此逝，江海寄余生。"));
    }
}

TEST(Core, DetectEncoding) {
    // MemoryLeakDetection mld;

    fun();
}

TEST(Core, DetectEncodingMulti) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    std::string expectedFileName = std::string(SmartCharsetConverter_TEST_DIR) + "/expected.txt";
    std::ifstream ifs(expectedFileName);
    if (!ifs.is_open()) {
        FAIL();
    }

    std::unordered_map<std::string, std::string> table;
    std::string line;
    while (std::getline(ifs, line)) {
        std::stringstream ss(line);
        std::string basename, encoding;
        ss >> basename >> encoding;
        std::string filename = std::string(SmartCharsetConverter_TEST_DIR) + "/" + basename;
        table[filename] = encoding;
    }

    CoreInitOption opt;
    Core core(L"temp.json", opt);

    for (auto [filename, expectedEncoding] : table) {
        auto [buf, len] = ReadFileToBuffer(utf8_to_wstring(filename));
        auto charsetCode = DetectEncoding(core.GetUCharDet().get(), buf.get(), len);

        auto got = to_utf8(ToViewCharsetName(charsetCode));

        if (got == expectedEncoding) {
            SetConsoleColor(ConsoleColor::GREEN);
        } else {
            SetConsoleColor(ConsoleColor::RED);
        }
        std::cout << std::string(20, '=') << std::endl;
        std::cout << "file: " << filename << std::endl;
        std::cout << "detect: " << to_utf8(ToViewCharsetName(charsetCode)) << std::endl;
        std::cout << "expected: " << expectedEncoding << std::endl;
        std::cout << std::endl;
        EXPECT_EQ(got, expectedEncoding);

        SetConsoleColor();
    }
}