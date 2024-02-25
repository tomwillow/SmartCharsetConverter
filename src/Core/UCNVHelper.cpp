#include "Core.h"

#include "Language.h"

void DealWithUCNVError(UErrorCode err) {
    switch (err) {
    case U_ZERO_ERROR:
        break;
    case U_AMBIGUOUS_ALIAS_WARNING: // windows-1252 时会出这个，暂时忽略
        break;
    case U_INVALID_CHAR_FOUND:
        throw std::runtime_error(GetLanguageService().GetUtf8String(StringId::INVALID_CHARACTERS));
    case U_ILLEGAL_CHAR_FOUND:
        throw std::runtime_error(GetLanguageService().GetUtf8String(StringId::INVALID_CHARACTERS));
    default:
        throw std::runtime_error("ucnv error. code=" + std::to_string(err));
        break;
    }
}