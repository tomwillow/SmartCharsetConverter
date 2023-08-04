#define _CRT_SECURE_NO_WARNINGS
#include "TimeStamp.h"

#include <cassert>

using namespace std;

#define TICKS_PER_SECOND 10000000
#define EPOCH_DIFFERENCE 11644473600LL

// Windows时间从1601-01-01T00：00：00Z开始。离UNIX/Linux时间(1970-01-01T00：00：00Z)还有11644473600秒。Windows时间为100纳秒。
// 因此，从UNIX时间获得秒的函数如下所示
time_t convertWindowsTimeToUnixTime(long long input) {
    long long temp;
    temp = input / TICKS_PER_SECOND; //convert from 100ns intervals to seconds;
    temp = temp - EPOCH_DIFFERENCE;  //subtract number of seconds between epochs
    return (time_t)temp;
}


TimeStamp::TimeStamp() : tp(selected_clock::now())
{

}

TimeStamp::TimeStamp(std::chrono::time_point<selected_clock> tp):
tp(tp)
{

}

TimeStamp::TimeStamp(time_t t)
{
    tp = selected_clock::from_time_t(t);
    //assert(selected_clock::to_time_t(tp) == t);
}

TimeStamp::TimeStamp(FILETIME fileTime)
{
    time_t t = convertWindowsTimeToUnixTime(*((long long *)&fileTime));
    tp = selected_clock::from_time_t(t);
}

bool TimeStamp::operator<(const TimeStamp &other) const
{
    return tp.time_since_epoch().count() < other.tp.time_since_epoch().count();
}

bool TimeStamp::operator<=(const TimeStamp &other) const
{
    return tp.time_since_epoch().count() <= other.tp.time_since_epoch().count();
}

bool TimeStamp::operator>(const TimeStamp &other) const
{
    return tp.time_since_epoch().count() > other.tp.time_since_epoch().count();
}

bool TimeStamp::operator>=(const TimeStamp &other) const
{
    return tp.time_since_epoch().count() >= other.tp.time_since_epoch().count();
}

TimeStamp TimeStamp::operator+(const std::chrono::milliseconds &dura) const
{
    return tp+dura;
}

TimeStamp::selected_clock::time_point TimeStamp::Raw() const
{
    return tp;
}

std::tstring TimeStamp::ToTString() const
{
    const int bufSize = 64;
    char buf[bufSize];

    auto t = selected_clock::to_time_t(tp);
    tm tempTM;
    errno_t err=LOCALTIME_R(&tempTM,&t);
    strftime(buf, bufSize, "%Y-%m-%d %H:%M:%S", &tempTM);
    return to_tstring(buf);
}

TimeStamp TimeStamp::now()
{
    return TimeStamp();
}