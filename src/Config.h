#pragma once

// self
#include "CharsetCode.h"
#include "LineBreaks.h"

#include <tstring.h>

// standard lib
#include <unordered_set>

/**
 * @brief 配置信息
 */
struct Configuration {
    enum class FilterMode { NO_FILTER, SMART, ONLY_SOME_EXTANT };
    enum class OutputTarget { ORIGIN, TO_DIR };
    static std::unordered_set<CharsetCode> normalCharset;

    FilterMode filterMode;
    OutputTarget outputTarget;
    std::tstring includeRule = TEXT("h hpp c cpp cxx txt");
    std::tstring excludeRule;
    std::tstring outputDir;
    CharsetCode outputCharset;
    bool enableConvertLineBreaks;
    LineBreaks lineBreak;

    Configuration()
        : filterMode(FilterMode::SMART), outputTarget(OutputTarget::ORIGIN), outputCharset(CharsetCode::UTF8),
          lineBreak(LineBreaks::CRLF), enableConvertLineBreaks(false) {}

    static bool IsNormalCharset(CharsetCode charset) {
        return normalCharset.find(charset) != normalCharset.end();
    }
};