#pragma once

#include <Core/Messages.h>

// third party

// standard
#include <unordered_map>
#include <array>

const std::string StringIdVersion = "0.2";

namespace v0_2 {
enum class StringId {
    BEGIN = 100,

    // 序号
    INDEX,
    FILENAME,
    SIZE,
    ENCODING,
    LINE_BREAKS,
    TEXT_PIECE,
    MSGBOX_ERROR,
    FAILED_ADD_BELOW,
    REASON,
    NON_TEXT_OR_NO_DETECTED, // 10
    AND_SO_ON,
    TIPS_USE_NO_FILTER,
    PROMPT,
    NO_FILE_TO_CONVERT,
    INVALID_OUTPUT_DIR,
    SUCCEED_SOME_FILES,
    FAILED_CONVERT_BELOW,
    NO_DEAL_DUE_TO_CANCEL,
    CONVERT_RESULT,
    NOTICE_SHOW_AS_UTF8, // 20
    SUPPORT_FORMAT_BELOW,
    SEPERATOR_DESCRIPTION,
    NO_SPECIFY_FILTER_EXTEND,
    INVALID_EXTEND_FILTER,
    ALL_FILES,
    FAILED_TO_SET_CHARSET_MANUALLY,
    NO_MEMORY,
    CANCEL,
    START_CONVERT,
    INVALID_CHARACTERS, // 30
    WILL_LOST_CHARACTERS,
    NOT_SUPPORT_ENCODING,
    ADD_REDUNDANTLY,
    NO_DETECTED_ENCODING,
    FAILED_TO_WRITE_FILE,
    FILE_SIZE_OUT_OF_LIMIT,
    FAILED_TO_OPEN_FILE,
    FILE_LISTS,
    SET_FILTER_MODE,
    NO_FILTER, // 40
    SMART_FILE_DETECTION,
    USE_FILE_EXTENSION,
    ADD_FILES_OR_FOLDER,
    ADD_FILES,
    ADD_FOLDER,
    SET_OUTPUT,
    OUTPUT_TO_ORIGIN,
    OUTPUT_TO_FOLDER,
    SET_OUTPUT_CHARSET,
    OTHERS, // 50
    CHANGE_LINE_BREAKS,
    CLEAR_LISTS,
    OPEN_WITH_NOTEPAD,
    SPECIFY_ORIGIN_ENCODING,
    REMOVE,
    SELECT_FOLDER,

    END
};

constexpr std::array<std::pair<int, int>, 2> STRING_ID_RANGES = {
    std::pair<int, int>{static_cast<int>(MessageId::BEGIN), static_cast<int>(MessageId::END)},
    std::pair<int, int>{static_cast<int>(StringId::BEGIN), static_cast<int>(StringId::END)},
};

} // namespace v0_2