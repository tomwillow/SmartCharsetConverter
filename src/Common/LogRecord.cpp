#define _CRT_SECURE_NO_WARNINGS
#include "LogRecord.h"

#include <fstream>
#include <iostream>

#include <cassert>

using namespace std;

std::tstring LogRecord::filename;
std::mutex LogRecord::m;
std::tstring LogRecord::buf;

LogRecord logRecord;

std::string GetNowTimeStr() {
    const int bufSize = 64;
    char buf[bufSize];
    time_t t = time(0);
    tm tm;
    LOCALTIME_R(&tm, &t);
    strftime(buf, bufSize, "%Y-%m-%d %H:%M:%S", &tm);
    return string(buf);
}

void LogRecord::SetLogFileName(std::tstring in_filename) {
    filename = in_filename;

    ios::sync_with_stdio(false); // Linux gcc.
    locale::global(locale(""));
    setlocale(LC_CTYPE, ""); // MinGW gcc.
    tcout.imbue(locale(""));
}

void LogRecord::AddBuffer(std::tstring content) {
    assert(!filename.empty());

    lock_guard<mutex> lk(m);
    buf += content;
}

void LogRecord::flush() {
    assert(!filename.empty());

    lock_guard<mutex> lk(m);

    string timestamp = GetNowTimeStr() + " ";

    tcout << to_tstring(timestamp) << TEXT("") << buf;

    FILE *fp = nullptr;
    fopen_s(&fp, to_string(filename).c_str(), "ab+");
    if (fp) {
        fwrite(timestamp.c_str(), timestamp.size(), 1, fp);

        string utf8 = to_utf8(buf);

        fwrite(utf8.c_str(), utf8.size(), 1, fp);
        fclose(fp);
    }

    buf.clear();
}

LogRecord &operator<<(LogRecord &logRecord, HWND hWnd) {
    TCHAR buf[16];
    _stprintf(buf, TEXT("%0p"), hWnd);
    logRecord.AddBuffer(buf);
    return logRecord;
}

LogRecord &operator<<(LogRecord &logRecord, TimeStamp timeStamp) {
    logRecord.AddBuffer(timeStamp.ToTString());
    return logRecord;
}
