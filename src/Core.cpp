#include "Core.h"

#include <FileFunction.h>

#include <unicode/ucnv.h>
#include <boost/functional/hash.hpp>

#include <stdexcept>
#include <unordered_map>

#ifdef _DEBUG
#include <iostream>
#endif

#undef min
#undef max

template <typename T1, typename T2>
class doublemap
{
public:
	doublemap(std::initializer_list<std::pair<T1, T2>> &&lst)
	{
		for (std::pair<T1, T2> pr : lst)
		{
			assert(has(pr.first) == false);
			insert(std::forward<std::pair<T1, T2>>(pr));
		}
	}

	bool has(const T1 &t1)const
	{
		return t1ToT2.find(t1) != t1ToT2.end();
	}

	void insert(std::pair<T1, T2> &&pr)
	{
		t1ToT2.insert(pr);
		t2ToT1.insert(std::pair<T2, T1>{pr.second, pr.first});
	}

	const T2 &operator[](const T1 &t1) const
	{
		return t1ToT2.at(t1);
	}

	const T1 &operator[](const T2 &t2) const
	{
		return t2ToT1.at(t2);
	}

private:
	std::unordered_map<T1, T2> t1ToT2;
	std::unordered_map<T2, T1> t2ToT1;
};

using namespace std;

const doublemap<CharsetCode, tstring> charsetCodeMap = {
	{CharsetCode::UTF8,TEXT("UTF-8")},
	{CharsetCode::UTF8BOM,TEXT("UTF-8 BOM")},
	{CharsetCode::UTF16LE,TEXT("UTF-16LE")},
	{CharsetCode::UTF16BE,TEXT("UTF-16BE")},
	{CharsetCode::GB18030,TEXT("GB18030")}
};

std::string ToICUCharsetName(CharsetCode code)
{
	if (code == CharsetCode::UTF8BOM)
	{
		return "UTF-8";
	}
	return to_string(charsetCodeMap[code]);
}

Core::Core(std::tstring iniFileName) :iniFileName(iniFileName)
{
	ReadFromIni();

	det = unique_ptr<uchardet, std::function<void(uchardet *)>>(uchardet_new(), [](uchardet *det) { uchardet_delete(det); });

	char s[] = "上岛咖啡决定是否可接受的空间放";
	int ret = uchardet_handle_data(det.get(), s, sizeof(s));
	switch (ret)
	{
	case HANDLE_DATA_RESULT_NEED_MORE_DATA:
	case HANDLE_DATA_RESULT_DETECTED:
		break;
	case HANDLE_DATA_RESULT_ERROR:
		throw runtime_error("uchardet fail");
	}
	uchardet_data_end(det.get());

	auto charset = uchardet_get_charset(det.get());

	UErrorCode err;
	auto allNames = ucnv_openAllNames(&err);
	while (1)
	{
		auto name = uenum_next(allNames, nullptr, &err);
		if (name == nullptr)
		{
			break;
		}
		cout << name << endl;
	}
}

const Configuration &Core::GetConfig() const
{
	return config;
}

void Core::SetFilterMode(Configuration::FilterMode mode)
{
	config.filterMode = mode;
	WriteToIni();
}

void Core::EnableRule(bool includeOrExclude, bool enable)
{
	if (includeOrExclude)
	{
		config.enableIncludeRule = enable;
	}
	else
	{
		config.enableExcludeRule = enable;
	}
	WriteToIni();
}

void Core::SetFilterRule(bool includeOrExclude, std::tstring rule)
{
	// check


	if (includeOrExclude)
	{
		config.includeRule = rule;
	}
	else
	{
		config.excludeRule = rule;
	}
	WriteToIni();
}

void Core::SetOutputTarget(Configuration::OutputTarget outputTarget)
{
	config.outputTarget = outputTarget;
	WriteToIni();
}

void Core::SetOutputDir(std::tstring outputDir)
{
	config.outputDir = outputDir;
	WriteToIni();
}

void Core::SetOutputCharset(Configuration::OutputCharset outputCharset)
{
	config.outputCharset = outputCharset;
	WriteToIni();
}

tuple<unique_ptr<UChar[]>,int32_t> Decode(const char *str, size_t len, CharsetCode code)
{
	auto icuCharsetName = ToICUCharsetName(code);

	UErrorCode err = U_ZERO_ERROR;
	UConverter *conv = ucnv_open(to_string(icuCharsetName).c_str(), &err);
	if (err != U_ZERO_ERROR)
	{
		throw runtime_error("ucnv出错。code=" + to_string(err));
	}

	size_t cap = len * sizeof(UChar);
	unique_ptr<UChar[]> target(new UChar[cap]);

	int32_t retLen = ucnv_toUChars(conv, target.get(), cap, str, len, &err);
	if (err != U_ZERO_ERROR)
	{
		throw runtime_error("ucnv出错。code=" + to_string(err));
	}

	ucnv_close(conv);

	return make_tuple<unique_ptr<UChar[]>, int32_t>(std::move(target), std::move(retLen));
}

std::tstring Core::GetEncodingStr(std::tstring filename) const
{
	unique_ptr<char[]> buf;
	uint64_t bufSize;

	// 只读取100KB
	ReadFileToBuffer(filename, buf, bufSize, 100 * KB);

	uchardet_reset(det.get());
	int ret = uchardet_handle_data(det.get(), buf.get(), bufSize);
	switch (ret)
	{
	case HANDLE_DATA_RESULT_NEED_MORE_DATA:
	case HANDLE_DATA_RESULT_DETECTED:
		break;
	case HANDLE_DATA_RESULT_ERROR:
		throw runtime_error("uchardet fail");
	}

	uchardet_data_end(det.get());

	auto charset = string(uchardet_get_charset(det.get()));

	// filter
	CharsetCode code;
	if (charset == "ASCII" || charset == "ANSI")
	{
		code = CharsetCode::UTF8;
	}
	else if (charset == "UTF-8")
	{
		// 区分有无BOM
		if (bufSize > 3 && memcmp(buf.get(), UTF8BOM_DATA, 3) == 0)
		{
			code = CharsetCode::UTF8BOM;
		}
	}
	else if (charset == "UTF-16LE")
	{
		code = CharsetCode::UTF16LE;
	}
	else if (charset == "UTF-16BE")
	{
		code = CharsetCode::UTF16BE;
	}
	else if (charset == "GB18030")
	{
		code = CharsetCode::GB18030;
	}
	else
	{
		string info = "暂不支持：";
		info += charset;
		info += "，请联系作者。";
		throw runtime_error(info);
	}

	auto content = Decode(buf.get(), std::max(32ULL, bufSize), code);


	return charsetCodeMap[code];
}

void Core::ReadFromIni()
{
}

void Core::WriteToIni()
{
}

//UINT Configuration::ToWinCodePage(OutputCharset charset)
//{
//	switch (charset)
//	{
//	case OutputCharset::UTF8:
//		return CP_UTF8;
//	case OutputCharset::GB18030:
//		return CP_GB18030;
//	}
//}