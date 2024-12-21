#pragma once

// self
#include "CharsetCode.h"
#include "LineBreaks.h"
#include "Config.h"
#include "Vietnamese.h"

#include <Common/tstring.h>

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

#undef min
#undef max

/**
 * @brief 根据code的字符集解码字符串为unicode
 * @return u16string(UTF-16LE)
 * @exception UCNVError ucnv出错。code
 */
std::u16string Decode(std::string_view src, CharsetCode code);

/**
 * 根据code的字符集解码字符串为unicode。
 * 为了只输出部分解码结果，输入一个最大输入bytes数量的限制。
 * @param maxInputBytes 最大输入bytes数量。src的长度如果大于maxInputBytes，只有maxInputBytes数量的数据会送去解码。
 * @return u16string(UTF-16LE)
 * @exception UCNVError ucnv出错。code
 */
std::u16string DecodeToLimitBytes(std::string_view src, uint64_t maxInputBytes, CharsetCode code);

/**
 * @brief 把unicode串编码为指定字符集
 * @param src u16string(UTF-16LE)
 * @return std::string CAUTION: this string is only as a container of char[] with the charset of targetCode.
 *          NOT mean its charset is ASCII or ANSI or others.
 * @exception viet::ConvertError
 * @exception UnassignedCharError 出现了不能转换的字符
 * @exception std::runtime_error ucnv出错
 */
std::string Encode(std::u16string_view src, CharsetCode targetCode);

struct ConvertParam {
    CharsetCode originCode;
    CharsetCode targetCode;
    bool doConvertLineBreaks;
    LineBreaks targetLineBreak; // target line break. if doConvertLineBreaks is false, this variable will be ignored.
};

/**
 * Convert encoding.
 * @exception viet::ConvertError
 * @exception UnassignedCharError 出现了不能转换的字符
 * @exception std::runtime_error ucnv出错
 */
std::string Convert(std::string_view src, ConvertParam inputParam);

viet::Encoding CharsetCodeToVietEncoding(CharsetCode code) noexcept;

struct CoreInitOption {

    std::function<void(int index, std::wstring filename, std::wstring fileSizeStr, std::wstring charsetStr,
                       std::wstring lineBreakStr, std::u16string textPiece)>
        fnUIUpdateItem = [](int index, std::wstring filename, std::wstring fileSizeStr, std::wstring charsetStr,
                            std::wstring lineBreakStr, std::u16string textPiece) {};
};

class Core {
public:
    Core(std::tstring configFileName, CoreInitOption opt);

    const Configuration &GetConfig() const;

    const std::unique_ptr<uchardet, std::function<void(uchardet *)>> &GetUCharDet() const;

    void SetFilterMode(Configuration::FilterMode mode);
    void SetFilterRule(const std::string &rule);

    void SetOutputTarget(Configuration::OutputTarget outputTarget);
    void SetOutputDir(const std::string &outputDir);
    void SetOutputCharset(CharsetCode outputCharset);
    void SetEnableConvertLineBreak(bool enableLineBreaks);
    void SetLineBreaks(LineBreaks lineBreak);
    void SetLanguage(const std::string &language) noexcept;

    struct AddItemResult {
        bool isIgnore = true; // 是否应该忽略掉
        uint64_t filesize;
        CharsetCode srcCharset;
        LineBreaks srcLineBreak;
        std::u16string strPiece;
    };

    /**
     * 加入一个文件到列表。
     * @exception runtime_error 重复添加
     * @exception file_io_error 读文件失败
     * @exception runtime_error ucnv出错。code
     * @exception io_error_ignore 按照配置忽略掉这个文件
     */
    [[nodiscard]] AddItemResult AddItem(const std::tstring &filename,
                                        const std::unordered_set<std::tstring> &filterDotExts);

    /**
     * 指定文件的字符集。
     * @exception file_io_error 读文件失败
     * @exception runtime_error ucnv出错。code
     */
    void SpecifyItemCharset(int index, const std::tstring &filename, CharsetCode code);

    void RemoveItem(const std::tstring &filename);

    void Clear();

    struct ConvertFileResult {
        std::tstring outputFileName;
        std::optional<std::string> errInfo;
        LineBreaks targetLineBreaks;
        int outputFileSize;
    };

    /**
     * @brief 转换一个文件。
     * @return <输出文件的文件名, 出错信息>
     */
    ConvertFileResult Convert(const std::tstring &inputFilename, CharsetCode originCode,
                              LineBreaks originLineBreak) noexcept;

private:
    std::tstring configFileName;
    CoreInitOption opt;
    Configuration config;
    std::unique_ptr<uchardet, std::function<void(uchardet *)>> det;

    std::unordered_set<std::tstring> listFileNames; // 当前列表中的文件

    void ReadConfigFromFile();

    void WriteConfigToFile();
};
