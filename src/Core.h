#pragma once

// self
#include "CharsetCode.h"
#include "LineBreaks.h"
#include "Config.h"

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

#undef min
#undef max

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

class io_error_ignore : public std::runtime_error {
public:
    io_error_ignore() : runtime_error("ignored") {}
};

struct CoreInitOption {

    std::function<void(int index, std::wstring filename, std::wstring fileSizeStr, std::wstring charsetStr,
                       std::wstring lineBreakStr, std::wstring textPiece)>
        fnUIUpdateItem = [](int index, std::wstring filename, std::wstring fileSizeStr, std::wstring charsetStr,
                            std::wstring lineBreakStr, std::wstring textPiece) {};
};

class Core {
public:
    Core(std::tstring configFileName, CoreInitOption opt);

    const Configuration &GetConfig() const;

    void SetFilterMode(Configuration::FilterMode mode);
    void SetFilterRule(const std::string &rule);

    void SetOutputTarget(Configuration::OutputTarget outputTarget);
    void SetOutputDir(const std::string &outputDir);
    void SetOutputCharset(CharsetCode outputCharset);
    void SetLineBreaks(LineBreaks lineBreak);
    void SetEnableConvertLineBreak(bool enableLineBreaks);

    //
    /**
     * @brief 读取最大100KB字节，返回编码集，Unicode文本，文本长度
     * @exception file_io_error 读文件失败
     * @exception runtime_error ucnv出错。code
     */
    std::tuple<CharsetCode, std::unique_ptr<UChar[]>, int> GetEncoding(const char *buf, int bufSize) const;

    struct AddItemResult {
        bool isIgnore = true; // 是否应该忽略掉
        uint64_t filesize;
        CharsetCode srcCharset;
        LineBreaks srcLineBreak;
        std::wstring strPiece;
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

    struct ConvertResult {
        std::tstring outputFileName;
        std::optional<std::tstring> errInfo;
        LineBreaks targetLineBreaks;
        int outputFileSize;
    };

    /**
     * @brief 转换一个文件。
     * @return <输出文件的文件名, 出错信息>
     */
    ConvertResult Convert(const std::tstring &inputFilename, CharsetCode originCode,
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
