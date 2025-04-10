#include "config.h"

#include "Core/Core.h"

#include <Common/FileFunction.h>
#include <Common/ConsoleSettings.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <unordered_map>
#include <random>

TEST(CoreVietnamese, ConvertToUtf8) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    std::string inputFilename = std::string(SmartCharsetConverter_TEST_DIR) + "/tcvn/demo1-tcvn.txt";
    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);

    ConvertParam param;
    param.originCode = CharsetCode::TCVN3;
    param.targetCode = CharsetCode::UTF8;
    param.doConvertLineBreaks = false;
    std::string utf8Str = Convert(std::string_view(buf.get(), bufSize), param);
    // WriteFileFromBuffer(std::string(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-got.txt", utf8Str.c_str(),
    //                    utf8Str.size());

    std::string expectFilename = std::string(SmartCharsetConverter_TEST_DIR) + "/tcvn/demo1-utf8.txt";
    auto [utf8Buf, utf8BufSize] = ReadFileToBuffer(expectFilename);
    std::string utf8ExpectStr(utf8Buf.get(), utf8BufSize);

    ASSERT_EQ(utf8Str.size(), utf8BufSize);
    ASSERT_EQ(utf8Str, utf8ExpectStr);
}

TEST(CoreVietnamese, ConvertToUtf16LE) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    std::string inputFilename = std::string(SmartCharsetConverter_TEST_DIR) + "/tcvn/demo1-tcvn.txt";
    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);

    ConvertParam param;
    param.originCode = CharsetCode::TCVN3;
    param.targetCode = CharsetCode::UTF16LE;
    param.doConvertLineBreaks = false;
    std::string ret = Convert(std::string_view(buf.get(), bufSize), param);
    std::u16string utf16LEStr;
    utf16LEStr.resize(ret.size() / sizeof(char16_t));
    memcpy(utf16LEStr.data(), ret.data(), ret.size());

    // WriteFileFromBuffer(std::string(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-got.txt", utf8Str.c_str(),
    //                    utf8Str.size());

    std::string expectFilename = std::string(SmartCharsetConverter_TEST_DIR) + "/tcvn/demo1-utf16le.txt";
    auto [utf16LEBuf, utf16LEBufSize] = ReadFileToBuffer(expectFilename);
    std::size_t utf16LEBufPsudoCharNums = utf16LEBufSize / sizeof(char16_t);
    std::u16string utf16LEExpectStr(reinterpret_cast<char16_t const *>(utf16LEBuf.get()), utf16LEBufPsudoCharNums);

    ASSERT_EQ(utf16LEStr.size(), utf16LEBufPsudoCharNums);
    ASSERT_EQ(utf16LEStr, utf16LEExpectStr);
}

TEST(CoreVietnamese, ConvertFromUtf8) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    std::string inputFilename = std::string(SmartCharsetConverter_TEST_DIR) + "/tcvn/demo1-utf8.txt";
    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);

    ConvertParam param;
    param.originCode = CharsetCode::UTF8;
    param.targetCode = CharsetCode::TCVN3;
    param.doConvertLineBreaks = false;
    std::string tcvn3StrGot = Convert(std::string_view(buf.get(), bufSize), param);
    // WriteFileFromBuffer(std::string(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-got.txt", utf8Str.c_str(),
    //                    utf8Str.size());

    std::string expectFilename = std::string(SmartCharsetConverter_TEST_DIR) + "/tcvn/demo1-tcvn.txt";
    auto [tcvn3BufExpected, tcvn3BufExpectedSize] = ReadFileToBuffer(expectFilename);
    std::string tcvn3StrExpected(tcvn3BufExpected.get(), tcvn3BufExpectedSize);

    ASSERT_EQ(tcvn3StrGot.size(), tcvn3BufExpectedSize);
    ASSERT_EQ(tcvn3StrGot, tcvn3StrExpected);
}

TEST(CoreVietnamese, ConvertFromUtf16LE) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    std::string inputFilename = std::string(SmartCharsetConverter_TEST_DIR) + "/tcvn/demo1-utf16le.txt";
    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);

    ConvertParam param;
    param.originCode = CharsetCode::UTF16LE;
    param.targetCode = CharsetCode::TCVN3;
    param.doConvertLineBreaks = false;
    std::string tcvn3StrGot = Convert(std::string_view((buf.get()), bufSize), param);
    // WriteFileFromBuffer(std::string(SmartCharsetConverter_TEST_DIR) + L"/tcvn/demo1-got.txt", utf8Str.c_str(),
    //                    utf8Str.size());

    std::string expectFilename = std::string(SmartCharsetConverter_TEST_DIR) + "/tcvn/demo1-tcvn.txt";
    auto [tcvn3BufExpected, tcvn3BufExpectedSize] = ReadFileToBuffer(expectFilename);
    std::string tcvn3StrExpected(tcvn3BufExpected.get(), tcvn3BufExpectedSize);

    ASSERT_EQ(tcvn3StrGot.size(), tcvn3BufExpectedSize);
    ASSERT_EQ(tcvn3StrGot, tcvn3StrExpected);
}

/**
 * @exception file_io_error
 *            ConvertError
 */
void TestBuiltinConvertOtherToOther(CharsetCode middleEncoding) {

    std::string inputFilename = std::string(SmartCharsetConverter_TEST_DIR) + "/tcvn/demo1-tcvn.txt";
    auto [buf, bufSize] = ReadFileToBuffer(inputFilename);

    std::string vniStr;

    ConvertParam param;
    param.originCode = CharsetCode::TCVN3;
    param.targetCode = middleEncoding;
    param.doConvertLineBreaks = false;
    EXPECT_NO_THROW(vniStr = Convert(std::string_view(buf.get(), bufSize), param));

    std::string tcvnStrGot;
    param.originCode = middleEncoding;
    param.targetCode = CharsetCode::TCVN3;
    EXPECT_NO_THROW(tcvnStrGot = Convert(vniStr, param));

    EXPECT_EQ(bufSize, tcvnStrGot.size());
    EXPECT_EQ(std::string(buf.get(), bufSize), tcvnStrGot);
}

TEST(CoreVietnamese, ConvertOtherToOther) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    TestBuiltinConvertOtherToOther(CharsetCode::VNI);
    TestBuiltinConvertOtherToOther(CharsetCode::VPS);
    TestBuiltinConvertOtherToOther(CharsetCode::VISCII);
}
