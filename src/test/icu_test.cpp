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

    UErrorCode err;
    UEnumeration *allNames = ucnv_openAllNames(&err);
    while (1) {
        auto name = uenum_next(allNames, nullptr, &err);
        if (name == nullptr) {
            break;
        }
    }
}