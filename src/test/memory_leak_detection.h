#pragma once

#ifdef WIN32
#include "memory_leak_detection_win.h"
#else
class MemoryLeakDetection final {
public:
    MemoryLeakDetection() {}
};
#endif
