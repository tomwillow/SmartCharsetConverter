#pragma once

#include "doublemap.h"

#include <tstring.h>

// third-party lib
#include <uchardet.h>
#include <unicode/ucnv.h>

// standard lib
#include <string>
#include <memory>
#include <functional>
#include <unordered_set>

enum class CharsetCode
{
	UTF8,
	UTF8BOM,
	UTF16BE,
	UTF16BEBOM,
	UTF16LE,
	UTF16LEBOM,
	GB18030,
	BIG5,
	SHIFT_JS,
	WINDOWS_1252,
	NOT_SUPPORTED,
	UNKNOWN

	// 添加字符集需要同步修改：charsetCodeMap ToICUCharsetName
};

// bom 字符
const char UTF8BOM_DATA[] = { 0xEF, 0xBB,0xBF };
const char UTF16LEBOM_DATA[] = { 0xFF, 0xFE };
const char UTF16BEBOM_DATA[] = { 0xFE, 0xFF };
const char UTF32LEBOM_DATA[] = { 0xFF, 0xFE,0,0 };
const char UTF32BEBOM_DATA[] = { 0xFE, 0xFF,0,0 };

std::tstring ToCharsetName(CharsetCode code);

// 编码集名字转CharsetCode。不含推测，只允许特定字符串出现。否则报assert
CharsetCode ToCharsetCode(const std::tstring &name);

bool HasBom(CharsetCode code);
const char *GetBomData(CharsetCode code);
int BomSize(CharsetCode code);

// 根据code的字符集解码字符串
std::tuple<std::unique_ptr<UChar[]>, int> Decode(const char *str, size_t len, CharsetCode code);

std::tuple<std::unique_ptr<char[]>, int> Encode(const std::unique_ptr<UChar[]> &buf, uint64_t bufSize, CharsetCode targetCode);

struct Configuration
{
	enum class FilterMode { NO_FILTER, SMART, ONLY_SOME_EXTANT };
	enum class OutputTarget { ORIGIN, TO_DIR };
	static std::unordered_set<CharsetCode> normalCharset;
	enum class LineBreaks { CRLF, LF, CR, EMPTY, MIX };

	FilterMode filterMode;
	OutputTarget outputTarget;
	std::tstring includeRule, excludeRule;
	std::tstring outputDir;
	CharsetCode outputCharset;
	bool enableConvertLineBreaks;
	LineBreaks lineBreak;

	Configuration() :
		filterMode(FilterMode::SMART),
		outputTarget(OutputTarget::ORIGIN),
		outputCharset(CharsetCode::UTF8),
		lineBreak(LineBreaks::CRLF),
		enableConvertLineBreaks(false)
	{
	}

	static bool IsNormalCharset(CharsetCode charset)
	{
		return normalCharset.find(charset) != normalCharset.end();
	}
};

// 识别换行符
Configuration::LineBreaks GetLineBreaks(const std::unique_ptr<UChar[]> &buf, int len);

// 变更换行符
void ChangeLineBreaks(std::unique_ptr<UChar[]> &buf, int &len, Configuration::LineBreaks targetLineBreak);

// LineBreaks类型到字符串的映射表
const doublemap<Configuration::LineBreaks, std::tstring> lineBreaksMap = {
	{Configuration::LineBreaks::CRLF,TEXT("CRLF")},
	{Configuration::LineBreaks::LF,TEXT("LF")},
	{Configuration::LineBreaks::CR,TEXT("CR")},
	{Configuration::LineBreaks::EMPTY,TEXT("")},
	{Configuration::LineBreaks::MIX,TEXT("混合")}
};


class Core
{
public:
	Core(std::tstring iniFileName);

	const Configuration &GetConfig() const;

	void SetFilterMode(Configuration::FilterMode mode);
	void SetFilterRule(std::tstring rule);

	void SetOutputTarget(Configuration::OutputTarget outputTarget);
	void SetOutputDir(std::tstring outputDir);
	void SetOutputCharset(CharsetCode outputCharset);
	void SetLineBreaks(Configuration::LineBreaks lineBreak);
	void SetEnableConvertLineBreak(bool enableLineBreaks);

	// 读取最大100KB字节，返回编码集，Unicode文本，文本长度
	std::tuple<CharsetCode, std::unique_ptr<UChar[]>, int> GetEncoding(std::tstring filename) const;

private:
	std::tstring iniFileName;
	Configuration config;
	std::unique_ptr<uchardet, std::function<void(uchardet *)>> det;

	void ReadFromIni();

	void WriteToIni();
};

