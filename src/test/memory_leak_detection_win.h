#pragma once

#ifdef WIN32

#include <windows.h>

#undef max
#undef min

#define _CRTDBG_MAP_ALLOC // to get more details
#include <stdlib.h>
#include <crtdbg.h> //for malloc and free

#include <gtest/gtest.h>

#include <iostream>
#include <cassert>

class MemoryLeakDetection final {
public:
    MemoryLeakDetection() {
        _CrtMemCheckpoint(&sOld); // take a snapshot
    }

    ~MemoryLeakDetection() {
        _CrtMemCheckpoint(&sNew);                    // take a snapshot
        if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
        {
            // OutputDebugString(TEXT("-----------_CrtMemDumpStatistics ---------"));
            //_CrtMemDumpStatistics(&sDiff);
            // OutputDebugString(TEXT("-----------_CrtMemDumpAllObjectsSince ---------"));
            //_CrtMemDumpAllObjectsSince(&sOld);
            // OutputDebugString(TEXT("-----------_CrtDumpMemoryLeaks ---------"));
            _CrtDumpMemoryLeaks();

            EXPECT_TRUE(0 && "Memory leak is detected! See debug output for detail.");
        }
    }

    void SetBreakAlloc(long index) const noexcept {
        (index);
        _CrtSetBreakAlloc(index);
    }

private:
    _CrtMemState sOld;
    _CrtMemState sNew;
    _CrtMemState sDiff;
};

#endif