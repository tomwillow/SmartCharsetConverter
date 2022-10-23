#pragma once

#include <tstring.h>

#include <uchardet.h>

#include <string>
#include <memory>
#include <functional>

enum class CharsetCode
{
	UTF8,
	UTF8BOM,
	UTF16BE,
	UTF16LE,
	GB18030,
	BIG5,
	SHIFT_JS,
	NOT_SUPPORTED
};

const char UTF8BOM_DATA[] = { 0xEF, 0xBB,0xBF };

// CharsetCode转tstring的名字
std::tstring to_tstring(CharsetCode code);

// 编码集名字转CharsetCode。不含推测，只允许特定字符串出现。否则报assert
CharsetCode ToCharsetCode(const std::tstring &name);

struct Configuration
{
	enum class FilterMode { SMART, MANUAL };
	enum class OutputTarget { ORIGIN, TO_DIR };
	enum class OutputCharset { UTF8, UTF8BOM, GB18030, OTHER_UNSPECIFIED, BIG5, SHIFT_JS };

	FilterMode filterMode;
	OutputTarget outputTarget;
	bool enableIncludeRule, enableExcludeRule;
	std::tstring includeRule, excludeRule;
	std::tstring outputDir;
	OutputCharset outputCharset;

	Configuration() :
		filterMode(FilterMode::SMART),
		outputTarget(OutputTarget::ORIGIN),
		outputCharset(OutputCharset::UTF8)
	{
	}

	static bool IsNormalCharset(OutputCharset charset)
	{
		return charset <= OutputCharset::GB18030;
	}
};

class Core
{
public:
	Core(std::tstring iniFileName);

	const Configuration &GetConfig() const;

	void SetFilterMode(Configuration::FilterMode mode);
	void EnableRule(bool includeOrExclude, bool enable);
	void SetFilterRule(bool includeOrExclude, std::tstring rule);

	void SetOutputTarget(Configuration::OutputTarget outputTarget);
	void SetOutputDir(std::tstring outputDir);
	void SetOutputCharset(Configuration::OutputCharset outputCharset);

	std::tstring GetEncodingStr(std::tstring filename) const;

private:
	std::tstring iniFileName;
	Configuration config;
	std::unique_ptr<uchardet,std::function<void(uchardet*)>> det;

	void ReadFromIni();

	void WriteToIni();
};

