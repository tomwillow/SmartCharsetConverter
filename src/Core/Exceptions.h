#pragma once

#include "Vietnamese.h"
#include "Messages.h"

#include <fmt/format.h>
#include <unicode/utypes.h>

#include <stdexcept>

class MyRuntimeError : public std::runtime_error {
public:
    MyRuntimeError(MessageId mid) : std::runtime_error(MessageIdToBasicString(mid)), mid(mid) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return translator->MessageIdToString(mid);
    }

protected:
    MessageId mid;
};

/**
 * 不可分配字符错误
 * 用于转换时出现不能转换到指定编码的情形。
 * err.what()方法会返回不能转换的字符组成的字符串(utf-8编码)。
 */
class UnassignedCharError : public MyRuntimeError {
public:
    UnassignedCharError(const std::string &unassignedChars) noexcept
        : MyRuntimeError(MessageId::WILL_LOST_CHARACTERS), unassignedChars(unassignedChars) {}

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
    UCNVError(int errCode) noexcept : MyRuntimeError(MessageId::UCNV_ERROR), errCode(errCode) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(mid), errCode);
    }

private:
    int errCode;
};

class TruncatedCharFoundError : UCNVError {
public:
    TruncatedCharFoundError() : UCNVError(U_TRUNCATED_CHAR_FOUND) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(MessageId::TRUNCATED_CHAR_FOUND));
    }
};

class ConvertError : public MyRuntimeError {
public:
    ConvertError(std::string content, int position, viet::Encoding srcEncoding, viet::Encoding destEncoding) noexcept
        : MyRuntimeError(MessageId::VIETNAMESE_CONVERT_ERROR), content(content), position(position),
          srcEncoding(srcEncoding), destEncoding(destEncoding) {}

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(mid), to_string(srcEncoding), to_string(srcEncoding), position,
                           content);
    }

private:
    std::string content;
    int position;
    viet::Encoding srcEncoding;
    viet::Encoding destEncoding;
};

class FileIOError : public MyRuntimeError {
public:
    FileIOError(MessageId mid, const std::string &filename) noexcept : MyRuntimeError(mid), filename(filename) {
        assert(mid == MessageId::FAILED_TO_WRITE_FILE || mid == MessageId::FILE_SIZE_OUT_OF_LIMIT ||
               mid == MessageId::FAILED_TO_OPEN_FILE);
    }

    virtual const std::string ToLocalString(TranslatorBase *translator) const noexcept {
        return fmt::format(translator->MessageIdToString(mid), filename);
    }

private:
    std::string filename;
};