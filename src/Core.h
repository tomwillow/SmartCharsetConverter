#pragma once

#include <tstring.h>

#include <uchardet.h>

#include <string>
#include <memory>
#include <functional>

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

	//static UINT ToWinCodePage(OutputCharset charset);
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

