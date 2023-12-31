#pragma once

// self
#include "CharsetCode.h"
#include "LineBreaks.h"

#include <tstring.h>

// third party
#include <nlohmann/json.hpp>

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
    std::string includeRule = u8"h hpp c cpp cxx txt";
    std::string excludeRule;
    std::string outputDir;
    CharsetCode outputCharset;
    bool enableConvertLineBreaks;
    LineBreaks lineBreak;
    std::string language;
    // if member variables is added, it must be synchronized at NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE below.

    Configuration()
        : filterMode(FilterMode::SMART), outputTarget(OutputTarget::ORIGIN), outputCharset(CharsetCode::UTF8),
          lineBreak(LineBreaks::CRLF), enableConvertLineBreaks(false) {}

    static bool IsNormalCharset(CharsetCode charset) {
        return normalCharset.find(charset) != normalCharset.end();
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Configuration, filterMode, outputTarget, includeRule, excludeRule, outputDir,
                                   outputCharset, enableConvertLineBreaks, lineBreak, language)