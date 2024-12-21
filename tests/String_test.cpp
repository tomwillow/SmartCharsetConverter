#include "config.h"

#include <Common/tstring.h>

#include <gtest/gtest.h>

TEST(Split, Split) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    struct Sample {
        std::wstring input;
        std::wstring sep;
        std::vector<std::wstring_view> expect;
    };

    std::vector<Sample> samples = {
        {TEXT(""), TEXT(" "), std::vector<std::tstring_view>{}},
        {TEXT("a"), TEXT(" "), std::vector<std::tstring_view>{TEXT("a")}},
        {TEXT("  a  "), TEXT(" "), std::vector<std::tstring_view>{TEXT("a")}},
        {TEXT("  a"), TEXT(" "), std::vector<std::tstring_view>{TEXT("a")}},
        {TEXT("a  "), TEXT(" "), std::vector<std::tstring_view>{TEXT("a")}},
        {TEXT("a b c"), TEXT(" "), std::vector<std::tstring_view>{TEXT("a"), TEXT("b"), TEXT("c")}},
        {TEXT(" a b c"), TEXT(" "), std::vector<std::tstring_view>{TEXT("a"), TEXT("b"), TEXT("c")}},
        {TEXT("a b c "), TEXT(" "), std::vector<std::tstring_view>{TEXT("a"), TEXT("b"), TEXT("c")}},
        {TEXT(" a b c "), TEXT(" "), std::vector<std::tstring_view>{TEXT("a"), TEXT("b"), TEXT("c")}},
        {TEXT("a\tb c\t"), TEXT(" \t"), std::vector<std::tstring_view>{TEXT("a"), TEXT("b"), TEXT("c")}},
    };

    for (auto &sample : samples) {
        ASSERT_EQ(Split(sample.input, sample.sep), sample.expect);
    }
}