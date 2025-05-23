#include "LineBreaks.h"
#include "Exceptions.h"

#include <stdexcept>

// LineBreaks类型到字符串的映射表
const doublemap<LineBreaks, std::string> lineBreaksMap = {
    {LineBreaks::CRLF, u8"CRLF"}, {LineBreaks::LF, u8"LF"},          {LineBreaks::CR, u8"CR"},
    {LineBreaks::EMPTY, u8""},    {LineBreaks::MIX, u8"N/A(Mixed)"}, {LineBreaks::UNKNOWN, u8"Unknown"}};

std::string LineBreaksToViewName(LineBreaks linebreaks) noexcept {
    return lineBreaksMap.at(linebreaks);
}

LineBreaks ViewNameToLineBreaks(std::string viewName) noexcept {
    return lineBreaksMap.at(viewName);
}

LineBreaks GetLineBreaks(const UChar *buf, std::size_t len) {
    LineBreaks ans = LineBreaks::EMPTY;
    for (std::size_t i = 0; i < len;) {
        const UChar &c = buf[i];
        if (c == UChar(u'\r')) {
            // \r\n
            if (i < len && buf[i + 1] == UChar(u'\n')) {
                if (ans == LineBreaks::EMPTY) {
                    ans = LineBreaks::CRLF;
                } else {
                    if (ans != LineBreaks::CRLF) {
                        ans = LineBreaks::MIX;
                        return ans;
                    }
                }
                i += 2;
                continue;
            }

            // \r
            if (ans == LineBreaks::EMPTY) {
                ans = LineBreaks::CR;
            } else {
                if (ans != LineBreaks::CR) {
                    ans = LineBreaks::MIX;
                    return ans;
                }
            }
            i++;
            continue;
        }

        // \n
        if (c == UChar(u'\n')) {
            if (ans == LineBreaks::EMPTY) {
                ans = LineBreaks::LF;
            } else {
                if (ans != LineBreaks::LF) {
                    ans = LineBreaks::MIX;
                    return ans;
                }
            }
            i++;
            continue;
        }

        i++;
    }
    return ans;
}

void ChangeLineBreaks(std::u16string &str, LineBreaks targetLineBreak) {
    std::vector<UChar> out;
    std::size_t len = str.size();
    out.reserve(len);

    std::vector<UChar> lineBreak;
    switch (targetLineBreak) {
    case LineBreaks::CRLF:
        lineBreak = {u'\r', u'\n'};
        break;
    case LineBreaks::LF:
        lineBreak = {u'\n'};
        break;
    case LineBreaks::CR:
        lineBreak = {u'\r'};
        break;
    }

    for (int i = 0; i < len;) {
        UChar c = str[i];
        if (c == UChar(u'\r')) {
            // \r\n
            if (i < len && str[i + 1] == UChar(u'\n')) {
                out.insert(out.end(), lineBreak.begin(), lineBreak.end());
                i += 2;
                continue;
            }

            // \r
            out.insert(out.end(), lineBreak.begin(), lineBreak.end());
            i++;
            continue;
        }

        if (c == UChar(u'\n')) {
            out.insert(out.end(), lineBreak.begin(), lineBreak.end());
            i++;
            continue;
        }

        out.push_back(c);
        i++;
    }

    if (out.size() >= std::numeric_limits<int>::max()) {
        throw MyRuntimeError(MessageId::STRING_LENGTH_OUT_OF_LIMIT);
    }

    str.resize(out.size());
    memcpy(str.data(), out.data(), out.size() * sizeof(UChar));
    return;
}