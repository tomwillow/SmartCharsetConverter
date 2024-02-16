#include "config.h"

#include <Core/LineBreaks.h>

#include <gtest/gtest.h>

TEST(LineBreaks, LineBreaks) {
    std::u16string ws;

    ws = u"\r";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::CR);
    ws = u"\r\r";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::CR);
    ws = u"\r00\r";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::CR);
    ws = u"\r\r\n";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::MIX);

    ws = u"\n";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::LF);
    ws = u"\n\r";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::MIX);
    ws = u"\n\n";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::LF);
    ws = u"\n\n\r";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::MIX);

    ws = u"\r\n";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::CRLF);
    ws = u"\r\n\n";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::MIX);
    ws = u"\r\n\r";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::MIX);

    ws = u"\n\r";
    ASSERT_EQ(GetLineBreaks(ws.c_str(), ws.length()), LineBreaks::MIX);
}