#pragma once

#include <Windows.h>

DWORD GetParentPID();

bool ReleaseConsole();

bool RedirectConsoleIO();

bool AttachParentConsole(short minLength);
