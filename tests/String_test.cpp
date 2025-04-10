#include "config.h"

#include <Common/tstring.h>

#include <gtest/gtest.h>

TEST(Split, Split) {
    SetConsoleOutputCP(65001); // 设置代码页为UTF-8

    struct Sample {
        std::string input;
        std::string sep;
        std::vector<std::string_view> expect;
    };

    std::vector<Sample> samples = {
        Sample{u8"", u8" ", std::vector<std::string_view>{}},
        Sample{u8"a", u8" ", std::vector<std::string_view>{u8"a"}},
        Sample{u8"  a  ", u8" ", std::vector<std::string_view>{u8"a"}},
        Sample{u8"  a", u8" ", std::vector<std::string_view>{u8"a"}},
        Sample{u8"a  ", u8" ", std::vector<std::string_view>{u8"a"}},
        Sample{u8"a b c", u8" ", std::vector<std::string_view>{u8"a", u8"b", u8"c"}},
        Sample{u8" a b c", u8" ", std::vector<std::string_view>{u8"a", u8"b", u8"c"}},
        Sample{u8"a b c ", u8" ", std::vector<std::string_view>{u8"a", u8"b", u8"c"}},
        Sample{u8" a b c ", u8" ", std::vector<std::string_view>{u8"a", u8"b", u8"c"}},
        Sample{u8"a\tb c\t", u8" \t", std::vector<std::string_view>{u8"a", u8"b", u8"c"}},
    };

    for (auto &sample : samples) {
        ASSERT_EQ(Split(sample.input, sample.sep), sample.expect);
    }
}