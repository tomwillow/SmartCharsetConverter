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

TEST(Core, icu) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    std::unordered_set<std::string> icuEncodings;

    UErrorCode err;
    UEnumeration *allNames = ucnv_openAllNames(&err);
    while (1) {
        auto name = uenum_next(allNames, nullptr, &err);
        if (name == nullptr) {
            break;
        }
        icuEncodings.insert(name);
        std::cout << name << std::endl;
    }

    std::vector<std::string> noICUNames;
    std::vector<std::runtime_error> errors;

    for (int i = static_cast<int>(CharsetCode::UTF8); i < static_cast<int>(CharsetCode::CHARSET_CODE_END); i++) {
        CharsetCode code = static_cast<CharsetCode>(i);

        if (GetConvertEngine(code) != ConvertEngine::ICU) {
            continue;
        }

        std::string icuName = ToICUCharsetName(code);
        if (icuEncodings.count(icuName) == 0) {
            std::cout << "Encoding \"" << icuName << "\" not found in icu supported name\n";

            try {
                Encode(u"abcdefg", code);
            } catch (const std::runtime_error &err) {
                std::cerr << err.what() << std::endl;
                errors.push_back(err);
            }
        }
    }

    ASSERT_TRUE(errors.empty());
}