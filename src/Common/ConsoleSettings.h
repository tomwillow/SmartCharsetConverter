#pragma once

#include <Windows.h>

enum class ConsoleColor { GREEN = 10, BLUE = 11, RED = 12, PINK = 13, YELLOW = 14, WHITE = 15 };

void SetConsoleColor(ConsoleColor color = ConsoleColor::WHITE) noexcept;