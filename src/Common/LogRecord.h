#pragma once

#include <tstring.h>
#include <TimeStamp.h>

#include <mutex>
#include <sstream>

class LogRecord
{
public:
	static void SetLogFileName(std::tstring filename);
	static void AddBuffer(std::tstring content);

	void flush();
private:
	static std::tstring filename;
	static std::mutex m;

	static std::tstring buf;
};

extern LogRecord logRecord;

// 指针
inline LogRecord &operator<<(LogRecord &logRecord, void *p)
{
	logRecord.AddBuffer(std::to_tstring((INT_PTR)p));
	return logRecord;
}

// 句柄
LogRecord &operator<<(LogRecord &logRecord, HWND hWnd);

// 时间戳
LogRecord &operator<<(LogRecord &logRecord, TimeStamp timeStamp);

// 宽字符串
inline LogRecord &operator<<(LogRecord &logRecord, const wchar_t s[])
{
	logRecord.AddBuffer(to_tstring(s));
	return logRecord;
}

// ansi字符串
inline LogRecord &operator<<(LogRecord &logRecord, const char s[])
{
	logRecord.AddBuffer(to_tstring(s));
	return logRecord;
}


inline LogRecord &operator<<(LogRecord &logRecord, const std::tstring &s)
{
	logRecord.AddBuffer(s);
	return logRecord;
}

template <typename T>
inline LogRecord &operator<<(LogRecord &logRecord, const T &t)
{
	logRecord.AddBuffer(std::to_tstring(t));
	return logRecord;
}

inline LogRecord &operator<<(LogRecord &logRecord, LogRecord &(*func)(LogRecord &))
{
	return func(logRecord);
}

inline LogRecord &endl(LogRecord &logRecord)
{
	logRecord.AddBuffer(TEXT("\r\n"));
	logRecord.flush();
	return logRecord;
}

#define LOG logRecord