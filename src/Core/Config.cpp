#include "Config.h"

std::unordered_set<CharsetCode> Configuration::normalCharset = {CharsetCode::UTF8, CharsetCode::UTF8BOM,
                                                                CharsetCode::GB18030};
