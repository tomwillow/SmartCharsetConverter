#pragma once

// self

// third-party lib
#include <unicode/ucnv.h>

// standard lib
#include <stdexcept>

/*
 * @exception UCNVError ucnv出错。code
 */
void DealWithUCNVError(UErrorCode err);