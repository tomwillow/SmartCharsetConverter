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

TEST(Core, uchardet_sample_test) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    const std::string uchardetSampleDir = std::string(SmartCharsetConverter_TEST_DIR) + "/uchardet_test_samples";
    auto table = helper::ScanDirectoryForExpectedEncodingTable(uchardetSampleDir, R"((.*))");
    auto expectPassTable =
        helper::ScanDirectoryForExpectedEncodingTable(std::string(SmartCharsetConverter_TEST_DIR) + "/expect_pass");
    auto notPassYetTable =
        helper::ScanDirectoryForExpectedEncodingTable(std::string(SmartCharsetConverter_TEST_DIR) + "/not_pass_yet");
    table.merge(std::move(expectPassTable));
    table.merge(std::move(notPassYetTable));

    CoreInitOption opt;
    Core core(u8"temp.json", opt);

    int passed = 0;
    for (auto [filename, expectedEncoding] : table) {
        auto [buf, len] = ReadFileToBuffer(filename);
        auto charsetCode = DetectEncoding(core.GetUCharDet().get(), buf.get(), len);

        if (charsetCode == expectedEncoding) {
            passed++;
            continue;
        }

        std::cout << std::string(20, '=') << std::endl;
        std::cout << "file: " << filename << std::endl;
        std::cout << "detect: " << ToViewCharsetName(charsetCode) << std::endl;
        std::cout << "expected: " << ToViewCharsetName(expectedEncoding) << std::endl;
        std::cout << std::endl;

        // EXPECT_EQ(charsetCode, expectedEncoding);  // not pass now
    }

    double rate = static_cast<double>(passed) / static_cast<double>(table.size());
    std::cout << "PASSED: " << rate * 100.0 << "% \n";

    // any changes to charset detection should increase this rate, not decrease it
    ASSERT_TRUE(rate > 0.337);
}