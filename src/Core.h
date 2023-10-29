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
#include <thread>
#include <stdexcept>
#include <optional>

enum class CharsetCode {
    UNKNOWN,
    EMPTY,
    NOT_SUPPORTED,

    //
    UTF8,
    UTF8BOM,

    UTF16BE,
    UTF16BEBOM,
    UTF16LE,
    UTF16LEBOM,
    UTF32BE,
    UTF32BEBOM,
    UTF32LE,
    UTF32LEBOM,

    GB18030,
    BIG5,
    SHIFT_JIS,
    EUC_JP,
    WINDOWS_1252,
    ISO_8859_1,

    CHARSET_CODE_END

    // 添加字符集需要同步修改：charsetCodeMap
};

struct MyCharset {
    std::tstring viewName;
    std::string icuName;
    std::unordered_set<std::string> icuNames;
};

// 字符集code到名称的映射表
const std::unordered_map<CharsetCode, MyCharset> charsetCodeMap = {
    {CharsetCode::UNKNOWN, MyCharset{TEXT("未知"), "-", {}}},
    {CharsetCode::EMPTY, MyCharset{TEXT("空"), "-", {}}},
    {CharsetCode::NOT_SUPPORTED, MyCharset{TEXT("不支持"), "-", {}}},
    {CharsetCode::UTF8, MyCharset{TEXT("UTF-8"), "UTF-8", {"ASCII", "ANSI"}}},
    {CharsetCode::UTF8BOM, MyCharset{TEXT("UTF-8 BOM"), "UTF-8", {}}},
    {CharsetCode::GB18030, MyCharset{TEXT("GB18030"), "GB18030", {}}},

    {CharsetCode::UTF16LE, MyCharset{TEXT("UTF-16LE"), "UTF-16LE", {}}},
    {CharsetCode::UTF16LEBOM, MyCharset{TEXT("UTF-16LE BOM"), "UTF-16LE", {}}},
    {CharsetCode::UTF16BE, MyCharset{TEXT("UTF-16BE"), "UTF-16BE", {}}},
    {CharsetCode::UTF16BEBOM, MyCharset{TEXT("UTF-16BE BOM"), "UTF-16BE", {}}},
    {CharsetCode::UTF32LE, MyCharset{TEXT("UTF-32LE"), "UTF-32LE", {}}},
    {CharsetCode::UTF32LEBOM, MyCharset{TEXT("UTF-32LE BOM"), "UTF-32LE", {}}},
    {CharsetCode::UTF32BE, MyCharset{TEXT("UTF-32BE"), "UTF-32BE", {}}},
    {CharsetCode::UTF32BEBOM, MyCharset{TEXT("UTF-32BE BOM"), "UTF-32BE", {}}},
    {CharsetCode::BIG5, MyCharset{TEXT("BIG5"), "Big5", {"Big5"}}},
    {CharsetCode::SHIFT_JIS, MyCharset{TEXT("SHIFT-JIS"), "SHIFT-JIS", {"SHIFT_JIS"}}},
    {CharsetCode::EUC_JP, MyCharset{TEXT("EUC-JP"), "EUC-JP", {"EUC-JP"}}},
    {CharsetCode::WINDOWS_1252, MyCharset{TEXT("WINDOWS-1252"), "WINDOWS-1252", {}}},
    {CharsetCode::ISO_8859_1, MyCharset{TEXT("ISO-8859-1"), "ISO-8859-1", {}}}};

std::tstring ToViewCharsetName(CharsetCode code);

// 编码集名字转CharsetCode。不含推测，只允许特定字符串出现。否则报assert
CharsetCode ToCharsetCode(const std::tstring &name);

// bom串
const char UTF8BOM_DATA[] = {'\xEF', '\xBB', '\xBF'};
const char UTF16LEBOM_DATA[] = {'\xFF', '\xFE'};
const char UTF16BEBOM_DATA[] = {'\xFE', '\xFF'};
const char UTF32LEBOM_DATA[] = {'\xFF', '\xFE', '\x0', '\x0'};
const char UTF32BEBOM_DATA[] = {'\xFE', '\xFF', '\x0', '\x0'};

bool HasBom(CharsetCode code);
const char *GetBomData(CharsetCode code);
int BomSize(CharsetCode code);

/**
 * @brief 返回buf的开头是否符合某种BOM，如果都不符合返回UNKNOWN
 */
CharsetCode CheckBom(char *buf, int bufSize);

/**
 * @brief 根据code的字符集解码字符串为unicode
 * @return 字符串指针，文本长度
 * @exception runtime_error ucnv出错。code
 */
std::tuple<std::unique_ptr<UChar[]>, int> Decode(const char *str, int len, CharsetCode code);

/**
 * @brief 把unicode串编码为指定字符集
 * @return 字符串指针，文本长度
 * @exception runtime_error ucnv出错。code
 */
std::tuple<std::unique_ptr<char[]>, int> Encode(const std::unique_ptr<UChar[]> &buf, int bufSize,
                                                CharsetCode targetCode);

/**
 * @brief 配置信息
 */
struct Configuration {
    enum class FilterMode { NO_FILTER, SMART, ONLY_SOME_EXTANT };
    enum class OutputTarget { ORIGIN, TO_DIR };
    static std::unordered_set<CharsetCode> normalCharset;
    enum class LineBreaks { CRLF, LF, CR, EMPTY, MIX, UNKNOWN };

    FilterMode filterMode;
    OutputTarget outputTarget;
    std::tstring includeRule, excludeRule;
    std::tstring outputDir;
    CharsetCode outputCharset;
    bool enableConvertLineBreaks;
    LineBreaks lineBreak;

    Configuration()
        : filterMode(FilterMode::SMART), outputTarget(OutputTarget::ORIGIN), outputCharset(CharsetCode::UTF8),
          lineBreak(LineBreaks::CRLF), enableConvertLineBreaks(false) {}

    static bool IsNormalCharset(CharsetCode charset) {
        return normalCharset.find(charset) != normalCharset.end();
    }
};

// 识别换行符
Configuration::LineBreaks GetLineBreaks(const UChar *buf, int len);

void Test_GetLineBreaks();

// 变更换行符
void ChangeLineBreaks(std::unique_ptr<UChar[]> &buf, int &len, Configuration::LineBreaks targetLineBreak);

// LineBreaks类型到字符串的映射表
const doublemap<Configuration::LineBreaks, std::tstring> lineBreaksMap = {
    {Configuration::LineBreaks::CRLF, TEXT("CRLF")}, {Configuration::LineBreaks::LF, TEXT("LF")},
    {Configuration::LineBreaks::CR, TEXT("CR")},     {Configuration::LineBreaks::EMPTY, TEXT("")},
    {Configuration::LineBreaks::MIX, TEXT("混合")},  {Configuration::LineBreaks::UNKNOWN, TEXT("未知")}};

class io_error_ignore : public std::runtime_error {
public:
    io_error_ignore() : runtime_error("ignored") {}
};

struct CoreInitOption {
    std::function<void(std::wstring filename, std::wstring fileSizeStr, std::wstring charsetStr,
                       std::wstring lineBreakStr, std::wstring textPiece)>
        fnUIAddItem = [](std::wstring filename, std::wstring fileSizeStr, std::wstring charsetStr,
                         std::wstring lineBreakStr, std::wstring textPiece) {};

    std::function<void(int index, std::wstring filename, std::wstring fileSizeStr, std::wstring charsetStr,
                       std::wstring lineBreakStr, std::wstring textPiece)>
        fnUIUpdateItem = [](int index, std::wstring filename, std::wstring fileSizeStr, std::wstring charsetStr,
                            std::wstring lineBreakStr, std::wstring textPiece) {};
};

class Core {
public:
    Core(std::tstring iniFileName, CoreInitOption opt);

    const Configuration &GetConfig() const;

    void SetFilterMode(Configuration::FilterMode mode);
    void SetFilterRule(std::tstring rule);

    void SetOutputTarget(Configuration::OutputTarget outputTarget);
    void SetOutputDir(std::tstring outputDir);
    void SetOutputCharset(CharsetCode outputCharset);
    void SetLineBreaks(Configuration::LineBreaks lineBreak);
    void SetEnableConvertLineBreak(bool enableLineBreaks);

    //
    /**
     * @brief 读取最大100KB字节，返回编码集，Unicode文本，文本长度
     * @exception file_io_error 读文件失败
     * @exception runtime_error ucnv出错。code
     */
    std::tuple<CharsetCode, std::unique_ptr<UChar[]>, int> GetEncoding(const char *buf, int bufSize) const;

    /**
     * 加入一个文件到列表。
     * @exception file_io_error 读文件失败
     * @exception runtime_error ucnv出错。code
     * @exception io_error_ignore 按照配置忽略掉这个文件
     */
    void AddItem(const std::tstring &filename, const std::unordered_set<std::tstring> &filterDotExts);

    /**
     * 指定文件的字符集。
     * @exception file_io_error 读文件失败
     * @exception runtime_error ucnv出错。code
     */
    void SpecifyItemCharset(int index, const std::tstring &filename, CharsetCode code);

    void RemoveItem(const std::tstring &filename);

    void Clear();

    struct ConvertResult {
        std::tstring outputFileName;
        std::optional<std::tstring> errInfo;
        Configuration::LineBreaks targetLineBreaks;
        int outputFileSize;
    };

    /**
     * @brief 转换一个文件。
     * @return <输出文件的文件名, 出错信息>
     */
    ConvertResult Convert(const std::tstring &inputFilename, CharsetCode originCode, CharsetCode targetCode,
                          Configuration::LineBreaks originLineBreak) noexcept;

private:
    std::tstring iniFileName;
    CoreInitOption opt;
    Configuration config;
    std::unique_ptr<uchardet, std::function<void(uchardet *)>> det;

    std::unordered_set<std::tstring> listFileNames; // 当前列表中的文件

    void ReadFromIni();

    void WriteToIni();
};
