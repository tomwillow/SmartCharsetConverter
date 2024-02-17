#include "config.h"

#include <Core/Core.h>
#include <Common/FileFunction.h>
#include <Common/ConsoleSettings.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <unordered_map>

TEST(Vietnamese, Convert) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    CoreInitOption opt;
    Core core(L"temp.json", opt);

    std::wstring inputFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-tcvn.txt";

    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);
    auto gotEncoding = core.DetectEncoding(buf.get(), bufSize);
    auto got = to_utf8(ToViewCharsetName(gotEncoding));
    std::string expectedEncoding = u8"TCVN3";
    EXPECT_EQ(got, expectedEncoding);

    // convert
    // core.Convert(inputFilename, )

    // if (got == expectedEncoding) {
    //    SetConsoleColor(ConsoleColor::GREEN);
    //} else {
    //    SetConsoleColor(ConsoleColor::RED);
    //}
    // std::cout << std::string(20, '=') << std::endl;
    // std::cout << "file: " << filename << std::endl;
    // std::cout << "detect: " << to_utf8(ToViewCharsetName(charsetCode)) << std::endl;
    // std::cout << "expected: " << expectedEncoding << std::endl;
    // std::cout << std::endl;

    // SetConsoleColor();
}