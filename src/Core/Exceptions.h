#pragma once

#include "Vietnamese.h"
#include "CharsetCode.h"
#include "Messages.h"

#include <Common/tstring.h>

#include <fmt/format.h>
#include <unicode/utypes.h>

#include <stdexcept>

class MyRuntimeError : public std::runtime_error {
public:
    MyRuntimeError(MessageId mid)
        : std::runtime_error(MessageIdToBasicString(mid)), mid(mid), errMsg(MessageIdToBasicString(mid)) {
#ifndef NDEBUG
        assert(errMsg.find("{") == std::string::npos);
#endif
    }

    MyRuntimeError(MessageId mid, const std::string errMsg) : std::runtime_error(errMsg), mid(mid), errMsg(errMsg) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return translator->MessageIdToString(mid);
    }

protected:
    MessageId mid;
    const std::string errMsg;
};

/**
 * 不可分配字符错误
 * 用于转换时出现不能转换到指定编码的情形。
 * err.what()方法会返回不能转换的字符组成的字符串(utf-8编码)。
 */
class UnassignedCharError : public MyRuntimeError {
public:
    UnassignedCharError(const std::string &unassignedChars) noexcept
        : MyRuntimeError(MessageId::WILL_LOST_CHARACTERS,
                         fmt::format(MessageIdToBasicString(MessageId::WILL_LOST_CHARACTERS), unassignedChars)),
          unassignedChars(unassignedChars) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(mid), unassignedChars);
    }

    const std::string GetUnassignedChar() const noexcept {
        return unassignedChars;
    }

private:
    std::string unassignedChars;
};

class io_error_ignore : public std::runtime_error {
public:
    io_error_ignore() : runtime_error("ignored") {}
};

class UCNVError : public MyRuntimeError {
public:
    UCNVError(int errCode) noexcept
        : MyRuntimeError(MessageId::UCNV_ERROR, fmt::format(MessageIdToBasicString(MessageId::UCNV_ERROR), errCode)),
          errCode(errCode) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(mid), errCode);
    }

private:
    int errCode;
};

class TruncatedCharFoundError : public UCNVError {
public:
    TruncatedCharFoundError() : UCNVError(U_TRUNCATED_CHAR_FOUND) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(MessageId::TRUNCATED_CHAR_FOUND));
    }
};

class IllegalCharFoundError : public UCNVError {
public:
    IllegalCharFoundError(CharsetCode decodeAs, std::size_t position, std::string corruptedDataPiece) noexcept
        : UCNVError(U_ILLEGAL_CHAR_FOUND), decodeAs(decodeAs), position(position),
          corruptedDataPiece(corruptedDataPiece) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(MessageId::CORRUPTED_DATA), ToViewCharsetName(decodeAs),
                           position, 32, to_hex(corruptedDataPiece));
    }

private:
    CharsetCode decodeAs;
    std::size_t position;
    std::string corruptedDataPiece; // the data at corrupted position, at least 32 bytes
};

using InvalidCharFoundError = IllegalCharFoundError;

class CharsetNotSupportedError : public MyRuntimeError {
public:
    CharsetNotSupportedError(CharsetCode targetCode)
        : MyRuntimeError(
              MessageId::CANNOT_CONVERT_CHARSET,
              fmt::format(MessageIdToBasicString(MessageId::CANNOT_CONVERT_CHARSET), ToViewCharsetName(targetCode))),
          targetCode(targetCode) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(mid), ToViewCharsetName(targetCode));
    }

private:
    CharsetCode targetCode;
};

class ConvertError : public MyRuntimeError {
public:
    ConvertError(std::string content, std::size_t position, viet::Encoding srcEncoding,
                 viet::Encoding destEncoding) noexcept
        : MyRuntimeError(MessageId::VIETNAMESE_CONVERT_ERROR,
                         fmt::format(MessageIdToBasicString(MessageId::VIETNAMESE_CONVERT_ERROR),
                                     to_string(srcEncoding), to_string(srcEncoding), position, content)),
          content(content), position(position), srcEncoding(srcEncoding), destEncoding(destEncoding) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(mid), to_string(srcEncoding), to_string(srcEncoding), position,
                           content);
    }

private:
    viet::Encoding srcEncoding;
    viet::Encoding destEncoding;
    std::size_t position;
    std::string content;
};

class FileIOError : public MyRuntimeError {
public:
    FileIOError(MessageId mid, const std::string &filename) noexcept
        : MyRuntimeError(mid, fmt::format(MessageIdToBasicString(mid), filename)), filename(filename) {
        assert(mid == MessageId::FAILED_TO_WRITE_FILE || mid == MessageId::FILE_SIZE_OUT_OF_LIMIT ||
               mid == MessageId::FAILED_TO_OPEN_FILE);
    }

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(mid), filename);
    }

private:
    std::string filename;
};