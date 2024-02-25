#pragma once

// self

// third-party lib
#include <unicode/ucnv.h>

// standard lib

/*
 * @exception runtime_error ucnv出错。code
 */
void DealWithUCNVError(UErrorCode err);