#pragma once

// self

// third-party lib
#include <unicode/ucnv.h>

// standard lib
#include <stdexcept>

class ucnv_error : public std::runtime_error {
public:
    ucnv_error(int errCode, const std::string &errMsg) noexcept : std::runtime_error(errMsg), errCode(errCode) {}

    int GetErrorCode() const noexcept {
        return errCode;
    }

private:
    int errCode;
};

/*
 * @exception ucnv_error ucnv出错。code
 */
void DealWithUCNVError(UErrorCode err);