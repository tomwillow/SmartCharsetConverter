#include "UCNVHelper.h"
#include "Exceptions.h"

#include "Language.h"

void DealWithUCNVError(UErrorCode err) {
    switch (err) {
    case U_ZERO_ERROR:
        break;
    case U_AMBIGUOUS_ALIAS_WARNING: // windows-1252 时会出这个，暂时忽略
        break;
    case U_INVALID_CHAR_FOUND:
        throw UCNVError(err, GetLanguageService().GetUtf8String(StringId::INVALID_CHARACTERS));
    case U_ILLEGAL_CHAR_FOUND:
        throw UCNVError(err, GetLanguageService().GetUtf8String(StringId::INVALID_CHARACTERS));
    default:
        throw UCNVError(err, "ucnv error. code=" + std::to_string(err));
        break;
    }
}