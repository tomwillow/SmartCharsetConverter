#pragma once

#include "TranslatorBase.h"

#include <cassert>

enum class MessageId {
    BEGIN = 0,
    WILL_LOST_CHARACTERS,       // "Some characters will be lost when converting to the target encoding: {}"
    INVALID_CHARACTERS,         // "Content contains invalid characters"
    UCNV_ERROR,                 // "UCNV error. error code: {}"
    VIETNAMESE_CONVERT_ERROR,   // "[{}->{}] convert error at position: {} with content: {}"
    TRUNCATED_CHAR_FOUND,       // "Truncated char found"
    ADD_REDUNDANTLY,            // "Duplicate addition"
    NO_DETECTED_ENCODING,       // "No encoding detected"
    FAILED_TO_WRITE_FILE,       // "Write failed: {}"
    FILE_SIZE_OUT_OF_LIMIT,     // "File size exceeds limit: {}"
    STRING_LENGTH_OUT_OF_LIMIT, // "String length exceeds limit"
    FAILED_TO_OPEN_FILE,        // "Failed to open file: {}"
    CORRUPTED_DATA, // "Corrupted data found while decode as {}. position: {} content(in hex, shown {} bytes at
                    // most): {}"
    END,
};

inline std::string MessageIdToBasicString(MessageId mid) noexcept {
    switch (mid) {
    case MessageId::WILL_LOST_CHARACTERS:
        return "Some characters will be lost when converting to the target encoding: {}";
    case MessageId::INVALID_CHARACTERS:
        return "Content contains invalid characters";
    case MessageId::UCNV_ERROR:
        return "UCNV error. error code: {}";
    case MessageId::VIETNAMESE_CONVERT_ERROR:
        return "[{}->{}] convert error at position: {} with content: {}";
    case MessageId::TRUNCATED_CHAR_FOUND:
        return "Truncated char found";
    case MessageId::ADD_REDUNDANTLY:
        return "Duplicate addition";
    case MessageId::NO_DETECTED_ENCODING:
        return "No encoding detected";
    case MessageId::FAILED_TO_WRITE_FILE:
        return "Write failed: {}";
    case MessageId::FILE_SIZE_OUT_OF_LIMIT:
        return "File size exceeds limit: {}";
    case MessageId::STRING_LENGTH_OUT_OF_LIMIT:
        return "String length exceeds limit";
    case MessageId::FAILED_TO_OPEN_FILE:
        return "Failed to open file: {}";
    case MessageId::CORRUPTED_DATA:
        return "Corrupted data found while decode as {}. position: {} content(in hex, shown {} bytes at most): "
               "{}";
    default:
        assert(0);
    }
    return "internal error";
}
