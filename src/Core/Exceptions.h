#pragma once

#include "Vietnamese.h"

#include <stdexcept>

/**
 * 不可分配字符错误
 * 用于转换时出现不能转换到指定编码的情形。
 * err.what()方法会返回不能转换的字符组成的字符串(utf-8编码)。
 */
class UnassignedCharError : public std::runtime_error {
public:
    UnassignedCharError(const std::string &unassignedChars) : std::runtime_error(unassignedChars) {}
};

class io_error_ignore : public std::runtime_error {
public:
    io_error_ignore() : runtime_error("ignored") {}
};

class UCNVError : public std::runtime_error {
public:
    UCNVError(int errCode, const std::string &errMsg) noexcept : std::runtime_error(errMsg), errCode(errCode) {}

    int GetErrorCode() const noexcept {
        return errCode;
    }

private:
    int errCode;
};

class ConvertError : public std::runtime_error {
public:
    ConvertError(std::string content, int position, viet::Encoding srcEncoding, viet::Encoding destEncoding) noexcept;

    virtual const char *what() const noexcept override {
        return errMsg.c_str();
    }

private:
    std::string content;
    int position;
    viet::Encoding srcEncoding;
    viet::Encoding destEncoding;
    std::string errMsg;
};