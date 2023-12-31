/*
    多语言支持

    加载流程：
    先从内置的json语言文件加载。
    然后加载外置的json语言文件。如果和内置的重复，那么覆盖内置的。

    等待配置读取当前语言，如果没有设置，
    那么读取系统语言。如果系统语言没有对应的语言包，那么加载英语。
*/

#pragma once

// third party
#include <nlohmann/json.hpp>

// standard
#include <unordered_map>
#include <functional>

enum class StringId {
    BEGIN = 0,
    // 序号
    INDEX,
    FILENAME,
    SIZE,
    ENCODING,
    LINE_BREAKS,
    TEXT_PIECE,
    MSGBOX_ERROR,
    FAILED_ADD_BELOW,
    REASON,
    NON_TEXT_OR_NO_DETECTED,
    AND_SO_ON,
    TIPS_USE_NO_FILTER,
    PROMPT,
    NO_FILE_TO_CONVERT,
    INVALID_OUTPUT_DIR,

    END
};

struct LanguagePack {
    std::string language;
    int langId;
    std::string author;
    std::string version;
    std::string date;
    std::unordered_map<StringId, std::string> data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LanguagePack, language, langId, author, version, date, data)

void CheckLanguagePack(const LanguagePack &langPack);

class LanguageService {
public:
    /**
     * @exception json解析失败抛出异常
     */
    LanguageService(std::function<std::string(void)> fnGetLanguageFromConfig);

    std::string GetCurrentLanguage() const noexcept;

    const std::string &GetUtf8String(StringId id) const noexcept;

    std::wstring GetWString(StringId id) const noexcept;

private:
    std::vector<std::string> avaliableLanguages;
    LanguagePack *currentLang;
    std::unordered_map<std::string, std::unique_ptr<LanguagePack>> languages;
    std::function<std::string(void)> fnGetLanguageFromConfig;

    void LoadLanguageNameFromInnerRCFile() noexcept;

    void LoadLanguageNameFromDir(const std::string &dir);

    bool HasLanguagePack(const std::string &lang) const noexcept;

    std::string GetLanguageNameByLangIdFromLoadedLanguages(int langId) const noexcept;
};

/**
 * @exception json解析失败抛出异常
 */
void InitLanguageService(std::function<std::string(void)> fnGetLanguageFromConfig);

void ReleaseLanguageService() noexcept;

LanguageService &GetLanguageService() noexcept;