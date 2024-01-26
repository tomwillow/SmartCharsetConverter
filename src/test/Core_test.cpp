#include "config.h"

#include <Core/Core.h>
#include <Common/FileFunction.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <unordered_map>

TEST(Core, DetectEncoding) {
    SetConsoleOutputCP(65001); // …Ë÷√¥˙¬Î“≥Œ™UTF-8

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
        auto charsetCode = core.DetectEncoding(buf.get(), len);

        auto got = to_utf8(ToViewCharsetName(charsetCode));

        std::cout << "file: " << filename << std::endl;
        std::cout << "detect: " << to_utf8(ToViewCharsetName(charsetCode)) << std::endl;
        std::cout << "expected: " << expectedEncoding << std::endl;
        std::cout << std::endl;
        EXPECT_EQ(got, expectedEncoding);
    }
}