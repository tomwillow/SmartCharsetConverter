#pragma once

#include <tstring.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

/*
防多开类

用法：
SingleApplication single;
if (single.IsRunning(TEXT("AppTitle")))
{
        throw runtime_error("只能同时运行一个实例！");
}

*/
class SingleApplication {
public:
    ~SingleApplication() { ReleaseMutex(mutex); }

    bool IsRunning(const std::tstring &mutex_name) {
        mutex = OpenMutex(MUTEX_ALL_ACCESS, 0, mutex_name.c_str());
        if (nullptr == mutex) {
            mutex = CreateMutex(0, 0, mutex_name.c_str());
            return false;
        } else {
            return true;
        }
    }

private:
    HANDLE mutex;
};