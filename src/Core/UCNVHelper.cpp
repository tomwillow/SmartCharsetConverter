#include "UCNVHelper.h"
#include "Exceptions.h"

void DealWithUCNVError(UErrorCode err) {
    switch (err) {
    case U_ZERO_ERROR:
        break;
    case U_AMBIGUOUS_ALIAS_WARNING: // windows-1252 时会出这个，暂时忽略
        break;
    // FIXME
    // case U_INVALID_CHAR_FOUND:
    //    throw UnassignedCharError(errStr);
    // case U_ILLEGAL_CHAR_FOUND:
    //    throw UnassignedCharError(errStr);
    default:
        throw UCNVError(err);
        break;
    }
}