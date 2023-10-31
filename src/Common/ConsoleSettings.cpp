#include "ConsoleSettings.h"

#include <cassert>

void SetConsoleColor(ConsoleColor color) noexcept {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    assert(hConsole);
    BOOL ok = SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
    assert(ok);
    HANDLE hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);
    assert(hConsoleErr);
    ok = SetConsoleTextAttribute(hConsoleErr, static_cast<WORD>(color));
    assert(ok);
}
