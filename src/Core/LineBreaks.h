#pragma once

// self
#include "doublemap.h"

#include <tstring.h>

// third-party lib
#include <unicode/ucnv.h>

#undef min
#undef max

enum class LineBreaks { CRLF, LF, CR, EMPTY, MIX, UNKNOWN };

// 识别换行符
LineBreaks GetLineBreaks(const UChar *buf, int len);

// 变更换行符
void ChangeLineBreaks(std::u16string &str, LineBreaks targetLineBreak);

// LineBreaks类型到字符串的映射表
const doublemap<LineBreaks, std::tstring> lineBreaksMap = {
    {LineBreaks::CRLF, TEXT("CRLF")}, {LineBreaks::LF, TEXT("LF")},    {LineBreaks::CR, TEXT("CR")},
    {LineBreaks::EMPTY, TEXT("")},    {LineBreaks::MIX, TEXT("混合")}, {LineBreaks::UNKNOWN, TEXT("未知")}};