#include "Core.h"

#include <FileFunction.h>

#include <unicode/ucnv.h>

#include <stdexcept>

#ifdef _DEBUG
#include <iostream>
#endif

#undef min
#undef max

using namespace std;

// 字符集code到名称的映射表
const doublemap<CharsetCode, tstring> charsetCodeMap = {
	{CharsetCode::UNKNOWN,TEXT("未知")},
	{CharsetCode::EMPTY,TEXT("空")},
	{CharsetCode::NOT_SUPPORTED,TEXT("不支持")},
	{CharsetCode::UTF8,TEXT("UTF-8")},
	{CharsetCode::UTF8BOM,TEXT("UTF-8 BOM")},
	{CharsetCode::UTF16LE,TEXT("UTF-16LE")},
	{CharsetCode::UTF16LEBOM,TEXT("UTF-16LE BOM")},
	{CharsetCode::UTF16BE,TEXT("UTF-16BE")},
	{CharsetCode::UTF16BEBOM,TEXT("UTF-16BE BOM")},
	{CharsetCode::GB18030,TEXT("GB18030")},
	{CharsetCode::WINDOWS_1252,TEXT("WINDOWS-1252")},
	{CharsetCode::ISO_8859_1,TEXT("ISO-8859-1")}
};

std::unordered_set<CharsetCode> Configuration::normalCharset = {
	CharsetCode::UTF8,CharsetCode::UTF8BOM,CharsetCode::GB18030
};

std::tstring ToCharsetName(CharsetCode code)
{
	return charsetCodeMap[code];
}

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

CharsetCode CheckBom(char *buf, int bufSize)
{
	if (bufSize >= sizeof(UTF8BOM_DATA) && memcmp(buf, UTF8BOM_DATA, sizeof(UTF8BOM_DATA)) == 0)
	{
		return CharsetCode::UTF8BOM;
	}
	if (bufSize >= sizeof(UTF16LEBOM_DATA) && memcmp(buf, UTF16LEBOM_DATA, sizeof(UTF16LEBOM_DATA)) == 0)
	{
		return CharsetCode::UTF8BOM;
	}
	if (bufSize >= sizeof(UTF16BEBOM_DATA) && memcmp(buf, UTF16BEBOM_DATA, sizeof(UTF16BEBOM_DATA)) == 0)
	{
		return CharsetCode::UTF8BOM;
	}
	if (bufSize >= sizeof(UTF32LEBOM_DATA) && memcmp(buf, UTF32LEBOM_DATA, sizeof(UTF32LEBOM_DATA)) == 0)
	{
		return CharsetCode::UTF8BOM;
	}
	if (bufSize >= sizeof(UTF32BEBOM_DATA) && memcmp(buf, UTF32BEBOM_DATA, sizeof(UTF32BEBOM_DATA)) == 0)
	{
		return CharsetCode::UTF8BOM;
	}
	return CharsetCode::UNKNOWN;
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

/*
* @exception runtime_error ucnv出错。code
*/
void DealWithUCNVError(UErrorCode err)
{
	switch (err)
	{
	case U_ZERO_ERROR:
		break;
	case U_AMBIGUOUS_ALIAS_WARNING:	// windows-1252 时会出这个，暂时忽略
		break;
	default:
		throw runtime_error("ucnv出错。code=" + to_string(err));
		break;
	}
}

tuple<unique_ptr<UChar[]>, int> Decode(const char *str, int len, CharsetCode code)
{
	if (code == CharsetCode::EMPTY)
	{
		return { nullptr, 0 };
	}

	// 从code转换到icu的字符集名称
	auto icuCharsetName = ToICUCharsetName(code);

	UErrorCode err = U_ZERO_ERROR;

	// 打开转换器
	UConverter *conv = ucnv_open(to_string(icuCharsetName).c_str(), &err);
	DealWithUCNVError(err);

	int32_t cap = len + 1;
	unique_ptr<UChar[]> target(new UChar[cap]);

	// 解码
	int retLen = ucnv_toUChars(conv, target.get(), cap, str, len, &err);
	DealWithUCNVError(err);

	ucnv_close(conv);

	return make_tuple<unique_ptr<UChar[]>, int32_t>(std::move(target), std::move(retLen));
}

std::tuple<std::unique_ptr<char[]>, int> Encode(const std::unique_ptr<UChar[]> &buf, int bufSize, CharsetCode targetCode)
{
	// 从code转换到icu的字符集名称
	auto icuCharsetName = ToICUCharsetName(targetCode);

	UErrorCode err = U_ZERO_ERROR;

	// 打开转换器
	UConverter *conv = ucnv_open(to_string(icuCharsetName).c_str(), &err);
	DealWithUCNVError(err);

	int32_t destCap = bufSize * sizeof(UChar) + 2;
	unique_ptr<char[]> target(new char[destCap]);

	// 解码
	int retLen;
	while (1)
	{
		err = U_ZERO_ERROR;
		retLen = ucnv_fromUChars(conv, target.get(), destCap, buf.get(), bufSize, &err);
		if (err == U_BUFFER_OVERFLOW_ERROR)
		{
			destCap = retLen  + 6; // 增加一个尾后0的大小：utf-8 单个字符最大占用字节数
			target.reset(new char[destCap]);
			continue;
		}
		DealWithUCNVError(err);
		if (err == U_ZERO_ERROR)
		{
			break;
		}
	}

	ucnv_close(conv);
	return make_tuple(std::move(target), retLen);
}

Configuration::LineBreaks GetLineBreaks(const unique_ptr<UChar[]> &buf, int len)
{
	Configuration::LineBreaks ans = Configuration::LineBreaks::EMPTY;
	for (int i = 0; i < len; )
	{
		UChar &c = buf.get()[i];
		if (c == UChar(u'\r'))
		{
			// \r\n
			if (i < len && buf.get()[i + 1] == UChar(u'\n'))
			{
				if (ans == Configuration::LineBreaks::EMPTY)
				{
					ans = Configuration::LineBreaks::CRLF;
				}
				else
				{
					if (ans != Configuration::LineBreaks::CRLF)
					{
						ans = Configuration::LineBreaks::MIX;
						return ans;
					}
				}
				i += 2;
				continue;
			}

			// \r
			if (ans == Configuration::LineBreaks::EMPTY)
			{
				ans = Configuration::LineBreaks::CR;
			}
			else
			{
				if (ans != Configuration::LineBreaks::CR)
				{
					ans = Configuration::LineBreaks::MIX;
					return ans;
				}
			}
			i++;
			continue;
		}

		// \n
		if (c == UChar(u'\n'))
		{
			if (ans == Configuration::LineBreaks::EMPTY)
			{
				ans = Configuration::LineBreaks::LF;
			}
			else
			{
				if (ans != Configuration::LineBreaks::LF)
				{
					ans = Configuration::LineBreaks::MIX;
					return ans;
				}
			}
			i++;
			continue;
		}

		i++;
	}
	return ans;
}

void ChangeLineBreaks(std::unique_ptr<UChar[]> &buf, int &len, Configuration::LineBreaks targetLineBreak)
{
	vector<UChar> out;
	out.reserve(len);

	vector<UChar> lineBreak;
	switch (targetLineBreak)
	{
	case Configuration::LineBreaks::CRLF:
		lineBreak = { u'\r',u'\n' };
		break;
	case Configuration::LineBreaks::LF:
		lineBreak = { u'\n' };
		break;
	case Configuration::LineBreaks::CR:
		lineBreak = { u'\r' };
		break;
	}

	for (int i = 0; i < len; )
	{
		UChar &c = buf.get()[i];
		if (c == UChar(u'\r'))
		{
			// \r\n
			if (i < len && buf.get()[i + 1] == UChar(u'\n'))
			{
				out.insert(out.end(), lineBreak.begin(), lineBreak.end());
				i += 2;
				continue;
			}

			// \r
			out.insert(out.end(), lineBreak.begin(), lineBreak.end());
			i++;
			continue;
		}

		if (c == UChar(u'\n'))
		{
			out.insert(out.end(), lineBreak.begin(), lineBreak.end());
			i++;
			continue;
		}

		out.push_back(c);
		i++;
	}

	if (out.size() >= std::numeric_limits<int>::max())
	{
		throw runtime_error("生成文件大小超出限制");
	}

	int outLen = static_cast<int>(out.size());
	buf.reset(new UChar[outLen]);
	memcpy(buf.get(), out.data(), out.size() * sizeof(UChar));
	len = outLen;

	return;
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

void Core::SetLineBreaks(Configuration::LineBreaks lineBreak)
{
	config.lineBreak = lineBreak;
	WriteToIni();
}

void Core::SetEnableConvertLineBreak(bool enableLineBreaks)
{
	config.enableConvertLineBreaks = enableLineBreaks;
}

std::tuple<CharsetCode, std::unique_ptr<UChar[]>, int32_t> Core::GetEncoding(std::tstring filename) const
{
	// 只读取100KB
	auto [buf, bufSize] = ReadFileToBuffer(filename, 100 * KB);

	if (bufSize == 0)
	{
		return { CharsetCode::EMPTY, nullptr, 0 };
	}

	if (bufSize >= std::numeric_limits<int>::max())
	{
		throw runtime_error("文件大小超出限制");
	}

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

	// 得到uchardet的识别结果
	string charset = string(uchardet_get_charset(det.get()));

	// filter
	CharsetCode code;
	if (charset == "ASCII" || charset == "ANSI")
	{
		code = CharsetCode::UTF8;
	}
	else if (charset == "UTF-8")
	{
		// 区分有无BOM
		if (bufSize >= sizeof(UTF8BOM_DATA) && memcmp(buf.get(), UTF8BOM_DATA, sizeof(UTF8BOM_DATA)) == 0)
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
		if (bufSize >= sizeof(UTF16LEBOM_DATA) && memcmp(buf.get(), UTF16LEBOM_DATA, sizeof(UTF16LEBOM_DATA)) == 0)
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
		if (bufSize >= sizeof(UTF16BEBOM_DATA) && memcmp(buf.get(), UTF16BEBOM_DATA, sizeof(UTF16BEBOM_DATA)) == 0)
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
	else if (charset == "WINDOWS-1252")
	{
		code = CharsetCode::WINDOWS_1252;
	}
	else if (charset == "ISO-8859-1")
	{
		code = CharsetCode::ISO_8859_1;
	}
	else if (charset == "")	// 没识别出来
	{
		code = CharsetCode::UNKNOWN;
		return make_tuple(code, nullptr, 0);
	}
	else
	{
		string info = "暂不支持：";
		info += charset;
		info += "，请联系作者。";
		throw runtime_error(info);
	}

	// 根据uchardet得出的字符集解码
	auto content = Decode(buf.get(), std::max(64, static_cast<int>(bufSize)), code);

	return make_tuple(code, std::move(get<0>(content)), get<1>(content));
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
