#include "Core.h"

#include <FileFunction.h>

#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>

#include <stdexcept>
#ifdef _DEBUG
#include <iostream>
#endif

using namespace std;

Core::Core(std::tstring iniFileName) :iniFileName(iniFileName)
{
	ReadFromIni();

	det = unique_ptr<uchardet, std::function<void(uchardet *)>>(uchardet_new(), [](uchardet *det) { uchardet_delete(det); });

	char s[] = "上岛咖啡决定是否可接受的空间放";
	int ret = uchardet_handle_data(det.get(), s, sizeof(s));
	if (ret != 0)
	{
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
	
	UConverter *conv = ucnv_open("GB18030", &err);

	size_t cap = sizeof(s) * 2;
	unique_ptr<UChar[]> target(new UChar[cap]);
	
	int32_t len = ucnv_toUChars(conv, target.get(), cap, s, strlen(s), &err);

	ucnv_close(conv);

	
	UCharsetDetector *csd=ucsdet_open(&err);
	ucsdet_setText(csd, s, sizeof(s), &err);
	const UCharsetMatch *ucm = ucsdet_detect(csd, &err);

	auto name=ucsdet_getName(ucm, &err);
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

std::tstring Core::GetEncodingStr(std::tstring filename) const
{
	// 只读取1MB

	unique_ptr<char[]> buf;
	uint64_t bufSize;

	ReadFileToBuffer(filename, buf, bufSize, 100 * KB);

	uchardet_reset(det.get());
	int failed = uchardet_handle_data(det.get(), buf.get(), bufSize);
	if (failed)
	{
		throw runtime_error("uchardet error");
	}

	uchardet_data_end(det.get());

	auto charset = uchardet_get_charset(det.get());

	//UErrorCode err;
	//UCharsetDetector *csd = ucsdet_open(&err);
	//ucsdet_setText(csd, buf.get(), bufSize, &err);
	//const UCharsetMatch *ucm = ucsdet_detect(csd, &err);

	//auto charset = ucsdet_getName(ucm, &err);
	//ucsdet_close(csd);

	return to_tstring(charset);
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