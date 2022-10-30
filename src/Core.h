#pragma once

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
	enum class FilterMode { SMART, ONLY_SOME_EXTANT };
	enum class OutputTarget { ORIGIN, TO_DIR };
	static std::unordered_set<CharsetCode> normalCharset;

	FilterMode filterMode;
	OutputTarget outputTarget;
	std::tstring includeRule, excludeRule;
	std::tstring outputDir;
	CharsetCode outputCharset;

	Configuration() :
		filterMode(FilterMode::SMART),
		outputTarget(OutputTarget::ORIGIN),
		outputCharset(CharsetCode::UTF8)
	{
	}

	static bool IsNormalCharset(CharsetCode charset)
	{
		return normalCharset.find(charset) != normalCharset.end();
	}
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

	std::tuple<std::tstring, std::unique_ptr<UChar[]>, int> GetEncodingStr(std::tstring filename) const;

private:
	std::tstring iniFileName;
	Configuration config;
	std::unique_ptr<uchardet, std::function<void(uchardet *)>> det;

	void ReadFromIni();

	void WriteToIni();
};

