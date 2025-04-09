#pragma once

// self
#include "doublemap.h"

#include <Common/tstring.h>

// third-party lib
#include <unicode/ucnv.h>

#undef min
#undef max

enum class LineBreaks { CRLF, LF, CR, EMPTY, MIX, UNKNOWN };

std::string LineBreaksToViewName(LineBreaks linebreaks) noexcept;

LineBreaks ViewNameToLineBreaks(std::string viewName) noexcept;

// 识别换行符
LineBreaks GetLineBreaks(const UChar *buf, std::size_t len);

// 变更换行符
void ChangeLineBreaks(std::u16string &str, LineBreaks targetLineBreak);
