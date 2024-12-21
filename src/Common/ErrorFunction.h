#pragma once

#include "tstring.h"

// 根据错误码返回对应的错误信息
std::tstring GetLastErrorString(DWORD errorCode);
