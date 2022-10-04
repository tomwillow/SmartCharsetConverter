#pragma once

#include <tstring.h>

#include <chrono>

#ifdef WIN32
#define LOCALTIME_R(pTM, pTimeT)  localtime_s(pTM, pTimeT)
#else
#define LOCALTIME_R(tm, ti)  localtime_r(ti, tm)
#endif
//#define localtime static_assert("localtime is not thread-safe. considering LOCALTIME_R")

template <typename T>
class TimeDuration
{
public:
    TimeDuration() {}

private:
    std::chrono::duration<T> d;
};


class TimeStamp
{
public:
    using selected_clock = std::chrono::system_clock;

    TimeStamp();
    TimeStamp(std::chrono::time_point<selected_clock> tp);
    TimeStamp(time_t t);
    TimeStamp(FILETIME fileTime);
    bool operator<(const TimeStamp &other) const;
    bool operator<=(const TimeStamp &other) const;
    bool operator>(const TimeStamp &other) const;
    bool operator>=(const TimeStamp &other) const;
    TimeStamp operator+(const std::chrono::milliseconds &dura) const;

    selected_clock::time_point Raw() const;

    std::tstring ToTString() const;

static TimeStamp now();
private:
    selected_clock::time_point tp;

};