#include "UnicodeTable.h"

namespace uni_table {

constexpr std::size_t UTF8_SPACE_SIZE = 0x10FFFF;
std::array<Utf8Str, UTF8_SPACE_SIZE + 1> utf8table;

Utf8Str PointToUtf8(int point) noexcept {
    Utf8Str utf8 = {{0, 0, 0, 0, 0, 0}};
    unsigned int codepoint = static_cast<unsigned int>(point);

    if (codepoint <= 0x7F) {
        // 1-byte sequence
        utf8[0] = static_cast<char>(codepoint);
        utf8[1] = '\0';
    } else if (codepoint <= 0x7FF) {
        // 2-byte sequence
        utf8[0] = static_cast<char>(0xC0 | (codepoint >> 6));
        utf8[1] = static_cast<char>(0x80 | (codepoint & 0x3F));
        utf8[2] = '\0';
    } else if (codepoint <= 0xFFFF) {
        // 3-byte sequence
        utf8[0] = static_cast<char>(0xE0 | (codepoint >> 12));
        utf8[1] = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        utf8[2] = static_cast<char>(0x80 | (codepoint & 0x3F));
        utf8[3] = '\0';
    } else if (codepoint <= 0x10FFFF) {
        // 4-byte sequence
        utf8[0] = static_cast<char>(0xF0 | (codepoint >> 18));
        utf8[1] = static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        utf8[2] = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        utf8[3] = static_cast<char>(0x80 | (codepoint & 0x3F));
        utf8[4] = '\0';
    } else {
        // Invalid codepoint, return empty string
        utf8[0] = '\0';
    }

    return utf8;
}

void InitUtf8Table() {
    for (int i = 1; i <= UTF8_SPACE_SIZE; ++i) {
        auto utf8 = PointToUtf8(i);
        utf8table[i] = utf8;
    }
}

// Functions

ExampleAssetsBrowser::ExampleAssetsBrowser() {
    AddItems(UTF8_SPACE_SIZE);
}

} // namespace uni_table