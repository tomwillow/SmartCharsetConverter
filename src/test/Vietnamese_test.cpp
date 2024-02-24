#include "config.h"

#include "Core/Vietnamese.h"

#include <Common/FileFunction.h>
#include <Common/ConsoleSettings.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <unordered_map>
#include <random>

TEST(Vietnamese, CheckEncoding) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8
    viet::Init();

    std::wstring inputFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-tcvn.txt";

    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);

    EXPECT_FALSE(viet::CheckEncoding(buf.get(), bufSize, viet::Encoding::VNI));
    EXPECT_FALSE(viet::CheckEncoding(buf.get(), bufSize, viet::Encoding::VPS));
    EXPECT_TRUE(viet::CheckEncoding(buf.get(), bufSize, viet::Encoding::VISCII));
    EXPECT_TRUE(viet::CheckEncoding(buf.get(), bufSize, viet::Encoding::TCVN3));
}

TEST(Vietnamese, BuiltinConvertToUtf8) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8
    viet::Init();

    std::wstring inputFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-tcvn.txt";
    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);
    std::string utf8Str = viet::ConvertToUtf8(std::string_view(buf.get(), bufSize), viet::Encoding::TCVN3);
    // WriteFileFromBuffer(utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-got.txt", utf8Str.c_str(),
    //                    utf8Str.size());

    std::wstring expectFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-utf8.txt";
    auto [utf8Buf, utf8BufSize] = ReadFileToBuffer(expectFilename);
    std::string utf8ExpectStr(utf8Buf.get(), utf8BufSize);

    ASSERT_EQ(utf8Str.size(), utf8BufSize);
    ASSERT_EQ(utf8Str, utf8ExpectStr);
}

TEST(Vietnamese, BuiltinConvertToUtf16LE) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8
    viet::Init();

    std::wstring inputFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-tcvn.txt";
    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);
    std::u16string utf16LEStr = viet::ConvertToUtf16LE(std::string_view(buf.get(), bufSize), viet::Encoding::TCVN3);
    // WriteFileFromBuffer(utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-got.txt", utf8Str.c_str(),
    //                    utf8Str.size());

    std::wstring expectFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-utf16le.txt";
    auto [utf16LEBuf, utf16LEBufSize] = ReadFileToBuffer(expectFilename);
    std::size_t utf16LEBufPsudoCharNums = utf16LEBufSize / sizeof(char16_t);
    std::u16string utf16LEExpectStr(reinterpret_cast<char16_t const *>(utf16LEBuf.get()), utf16LEBufPsudoCharNums);

    ASSERT_EQ(utf16LEStr.size(), utf16LEBufPsudoCharNums);
    ASSERT_EQ(utf16LEStr, utf16LEExpectStr);
}

TEST(Vietnamese, BuiltinConvertFromUtf8) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8
    viet::Init();

    std::wstring inputFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-utf8.txt";
    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);
    std::string tcvn3StrGot = viet::ConvertFromUtf8(std::string_view(buf.get(), bufSize), viet::Encoding::TCVN3);
    // WriteFileFromBuffer(utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-got.txt", utf8Str.c_str(),
    //                    utf8Str.size());

    std::wstring expectFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-tcvn.txt";
    auto [tcvn3BufExpected, tcvn3BufExpectedSize] = ReadFileToBuffer(expectFilename);
    std::string tcvn3StrExpected(tcvn3BufExpected.get(), tcvn3BufExpectedSize);

    ASSERT_EQ(tcvn3StrGot.size(), tcvn3BufExpectedSize);
    ASSERT_EQ(tcvn3StrGot, tcvn3StrExpected);
}

TEST(Vietnamese, BuiltinConvertFromUtf16LE) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8
    viet::Init();

    std::wstring inputFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-utf16le.txt";
    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);
    std::string tcvn3StrGot = viet::ConvertFromUtf16LE(
        std::u16string_view(reinterpret_cast<const char16_t *>(buf.get()), bufSize / sizeof(char16_t)),
        viet::Encoding::TCVN3);
    // WriteFileFromBuffer(utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-got.txt", utf8Str.c_str(),
    //                    utf8Str.size());

    std::wstring expectFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-tcvn.txt";
    auto [tcvn3BufExpected, tcvn3BufExpectedSize] = ReadFileToBuffer(expectFilename);
    std::string tcvn3StrExpected(tcvn3BufExpected.get(), tcvn3BufExpectedSize);

    ASSERT_EQ(tcvn3StrGot.size(), tcvn3BufExpectedSize);
    ASSERT_EQ(tcvn3StrGot, tcvn3StrExpected);
}

/**
 * @exception file_io_error
 *            ConvertError
 */
void TestBuiltinConvertOtherToOther(viet::Encoding middleEncoding) {
    viet::Init();

    std::wstring inputFilename = utf8_to_wstring(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-tcvn.txt";
    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);

    std::string vniStr;
    EXPECT_NO_THROW(vniStr =
                        viet::Convert(std::string_view(buf.get(), bufSize), viet::Encoding::TCVN3, middleEncoding));
    std::string tcvnStrGot;
    EXPECT_NO_THROW(tcvnStrGot = viet::Convert(vniStr, middleEncoding, viet::Encoding::TCVN3));

    EXPECT_EQ(bufSize, tcvnStrGot.size());
    EXPECT_EQ(std::string(buf.get(), bufSize), tcvnStrGot);
}

TEST(Vietnamese, BuiltinConvertOtherToOther) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    TestBuiltinConvertOtherToOther(viet::Encoding::VNI);
    TestBuiltinConvertOtherToOther(viet::Encoding::VPS);
    TestBuiltinConvertOtherToOther(viet::Encoding::VISCII);
}

TEST(Vietnamese, ConvertFuzz) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8
    viet::Init();

    const int count = 1024;
    std::string randUtf8Str;
    std::default_random_engine eng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> unif(0, viet::internal::TABLE_LENGTH - 1);
    for (int i = 0; i < count; ++i) {
        int index = unif(eng);
        randUtf8Str += viet::internal::utf8Table[index];
    }

    {
        std::string dialect = viet::ConvertFromUtf8(randUtf8Str, viet::Encoding::VNI);
        std::string gotUtf8 = viet::ConvertToUtf8(dialect, viet::Encoding::VNI);
        EXPECT_EQ(gotUtf8, randUtf8Str);
    }
    {
        std::string dialect = viet::ConvertFromUtf8(randUtf8Str, viet::Encoding::VPS);
        std::string gotUtf8 = viet::ConvertToUtf8(dialect, viet::Encoding::VPS);
        EXPECT_EQ(gotUtf8, randUtf8Str);
    }
    {
        std::string dialect = viet::ConvertFromUtf8(randUtf8Str, viet::Encoding::VISCII);
        std::string gotUtf8 = viet::ConvertToUtf8(dialect, viet::Encoding::VISCII);
        EXPECT_EQ(gotUtf8, randUtf8Str);
    }

    // TCVN3比较特殊，不能用上面的方法来测试。原因是TCVN3的映射表虽然有2字节的，但TCVN3并不是一个多字节字符集(MBCS)。
    // 所以，这里构造TCVN3的测试方法为：构造由随机的单字节TCVN3字符组成的字符串，再和UTF8互转。
    {
        std::string randTCVN3;
        std::uniform_int_distribution<int> unifASCII(32, 126);
        for (int i = 0; i < count; ++i) {
            int rn = unif(eng);
            if (rn % 2) {
                randTCVN3 += static_cast<char>(unifASCII(eng));
            } else {
                while (1) {
                    auto &tcvn3Word = viet::internal::tcvn3Table[rn];
                    if (tcvn3Word.size() == 1) {
                        randTCVN3 += tcvn3Word;
                        break;
                    }

                    rn = unif(eng);
                }
            }
        }

        std::string middleUtf8 = viet::ConvertToUtf8(randTCVN3, viet::Encoding::TCVN3);
        std::string gotTCVN3 = viet::ConvertFromUtf8(middleUtf8, viet::Encoding::TCVN3);
        EXPECT_EQ(gotTCVN3, randTCVN3);
    }

    //{
    //    std::string dialect = viet::ConvertFromUtf8(randUtf8Str, viet::Encoding::TCVN3);
    //    std::string gotUtf8 = viet::ConvertToUtf8(dialect.c_str(), dialect.size(), viet::Encoding::TCVN3);
    //    EXPECT_EQ(gotUtf8, randUtf8Str);
    //}
}