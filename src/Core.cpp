#include "Core.h"

#include <FileFunction.h>

#include <unicode/ucnv.h>

#include <stdexcept>
#include <unordered_map>
#include <cassert>

#ifdef _DEBUG
#include <iostream>
#endif

#undef min
#undef max

// 双向map，支持双向查表操作
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

// 字符集code到名称的映射表
const doublemap<CharsetCode, tstring> charsetCodeMap = {
	{CharsetCode::UTF8,TEXT("UTF-8")},
	{CharsetCode::UTF8BOM,TEXT("UTF-8 BOM")},
	{CharsetCode::UTF16LE,TEXT("UTF-16LE")},
	{CharsetCode::UTF16LEBOM,TEXT("UTF-16LE BOM")},
	{CharsetCode::UTF16BE,TEXT("UTF-16BE")},
	{CharsetCode::UTF16BEBOM,TEXT("UTF-16BE BOM")},
	{CharsetCode::GB18030,TEXT("GB18030")},
	{CharsetCode::UNKNOWN,TEXT("未知")}
};

std::unordered_set<CharsetCode> Configuration::normalCharset = {
	CharsetCode::UTF8,CharsetCode::UTF8BOM,CharsetCode::GB18030
};

CharsetCode ToCharsetCode(const std::tstring &name)
{
	return charsetCodeMap[name];
}

bool HasBom(CharsetCode code)
{
	switch (code)
	{
	case CharsetCode::UTF8BOM:
	case CharsetCode::UTF16LEBOM:
	case CharsetCode::UTF16BEBOM:
		return true;
	}
	return false;
}

const char *GetBomData(CharsetCode code)
{
	switch (code)
	{
	case CharsetCode::UTF8BOM:
		return UTF8BOM_DATA;
	case CharsetCode::UTF16LEBOM:
		return UTF16LEBOM_DATA;
	case CharsetCode::UTF16BEBOM:
		return UTF16BEBOM_DATA;
	}
	return nullptr;
}

int BomSize(CharsetCode code)
{
	switch (code)
	{
	case CharsetCode::UTF8BOM:
		return sizeof(UTF8BOM_DATA);
	case CharsetCode::UTF16LEBOM:
		return sizeof(UTF16LEBOM_DATA);
	case CharsetCode::UTF16BEBOM:
		return sizeof(UTF16BEBOM_DATA);
	}
	return 0;
}


std::string ToICUCharsetName(CharsetCode code)
{
	switch (code)
	{
	case CharsetCode::UTF8BOM:
		return "UTF-8";
	case CharsetCode::UTF16LEBOM:
		return "UTF-16LE";
	case CharsetCode::UTF16BEBOM:
		return "UTF-16BE";
	}
	return to_string(charsetCodeMap[code]);
}

// 根据code的字符集解码字符串
tuple<unique_ptr<UChar[]>, int> Decode(const char *str, size_t len, CharsetCode code)
{
	// 从code转换到icu的字符集名称
	auto icuCharsetName = ToICUCharsetName(code);

	UErrorCode err = U_ZERO_ERROR;

	// 打开转换器
	UConverter *conv = ucnv_open(to_string(icuCharsetName).c_str(), &err);
	if (err != U_ZERO_ERROR)
	{
		throw runtime_error("ucnv出错。code=" + to_string(err));
	}

	size_t cap = len + 1;
	unique_ptr<UChar[]> target(new UChar[cap]);

	// 解码
	int retLen = ucnv_toUChars(conv, target.get(), cap, str, len, &err);
	if (err != U_ZERO_ERROR)
	{
		throw runtime_error("ucnv出错。code=" + to_string(err));
	}

	ucnv_close(conv);

	return make_tuple<unique_ptr<UChar[]>, int32_t>(std::move(target), std::move(retLen));
}

std::tuple<std::unique_ptr<char[]>, int> Encode(const std::unique_ptr<UChar[]> &buf, uint64_t bufSize, CharsetCode targetCode)
{
	// 从code转换到icu的字符集名称
	auto icuCharsetName = ToICUCharsetName(targetCode);

	UErrorCode err = U_ZERO_ERROR;

	// 打开转换器
	UConverter *conv = ucnv_open(to_string(icuCharsetName).c_str(), &err);
	if (err != U_ZERO_ERROR)
	{
		throw runtime_error("ucnv出错。code=" + to_string(err));
	}

	size_t cap = bufSize * sizeof(UChar) + 2;
	unique_ptr<char[]> target(new char[cap]);

	// 解码
	int retLen = ucnv_fromUChars(conv, target.get(), cap, buf.get(), bufSize, &err);
	if (err != U_ZERO_ERROR)
	{
		throw runtime_error("ucnv出错。code=" + to_string(err));
	}

	ucnv_close(conv);
	return make_tuple(std::move(target), retLen);
}

Core::Core(std::tstring iniFileName) :iniFileName(iniFileName)
{
	// 读ini
	ReadFromIni();

	// 初始化uchardet
	det = unique_ptr<uchardet, std::function<void(uchardet *)>>(uchardet_new(), [](uchardet *det) { uchardet_delete(det); });

	//UErrorCode err;
	//auto allNames = ucnv_openAllNames(&err);
	//while (1)
	//{
	//	auto name = uenum_next(allNames, nullptr, &err);
	//	if (name == nullptr)
	//	{
	//		break;
	//	}
	//	cout << name << endl;
	//}
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

void Core::SetFilterRule(std::tstring rule)
{
	config.includeRule = rule;
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

void Core::SetOutputCharset(CharsetCode outputCharset)
{
	config.outputCharset = outputCharset;
	WriteToIni();
}

std::tuple<std::tstring, std::unique_ptr<UChar[]>, int32_t> Core::GetEncodingStr(std::tstring filename) const
{
	// 只读取100KB
	auto [buf, bufSize] = ReadFileToBuffer(filename, 100 * KB);

	// 用uchardet判定字符集
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
		if (bufSize > sizeof(UTF8BOM_DATA) && memcmp(buf.get(), UTF8BOM_DATA, sizeof(UTF8BOM_DATA)) == 0)
		{
			code = CharsetCode::UTF8BOM;
		}
		else
		{
			code = CharsetCode::UTF8;
		}
	}
	else if (charset == "UTF-16LE")
	{
		// 区分有无BOM
		if (bufSize > sizeof(UTF16LEBOM_DATA) && memcmp(buf.get(), UTF16LEBOM_DATA, sizeof(UTF16LEBOM_DATA)) == 0)
		{
			code = CharsetCode::UTF16LEBOM;
		}
		else
		{
			code = CharsetCode::UTF16LE;
		}
	}
	else if (charset == "UTF-16BE")
	{
		// 区分有无BOM
		if (bufSize > sizeof(UTF16BEBOM_DATA) && memcmp(buf.get(), UTF16BEBOM_DATA, sizeof(UTF16BEBOM_DATA)) == 0)
		{
			code = CharsetCode::UTF16BEBOM;
		}
		else
		{
			code = CharsetCode::UTF16BE;
		}
	}
	else if (charset == "GB18030")
	{
		code = CharsetCode::GB18030;
	}
	else if (charset == "")	// 没识别出来
	{
		code = CharsetCode::UNKNOWN;
		return make_tuple(charsetCodeMap[code], nullptr, 0);
	}
	else
	{
		string info = "暂不支持：";
		info += charset;
		info += "，请联系作者。";
		throw runtime_error(info);
	}

	// 根据uchardet得出的字符集解码
	auto content = Decode(buf.get(), std::max(64ULL, bufSize), code);

	return make_tuple(charsetCodeMap[code], std::move(get<0>(content)), get<1>(content));
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
