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
    NON_TEXT_OR_NO_DETECTED, // 10
    AND_SO_ON,
    TIPS_USE_NO_FILTER,
    PROMPT,
    NO_FILE_TO_CONVERT,
    INVALID_OUTPUT_DIR,
    SUCCEED_SOME_FILES,
    FAILED_CONVERT_BELOW,
    NO_DEAL_DUE_TO_CANCEL,
    CONVERT_RESULT,
    NOTICE_SHOW_AS_UTF8, // 20
    SUPPORT_FORMAT_BELOW,
    SEPERATOR_DESCRIPTION,
    NO_SPECIFY_FILTER_EXTEND,
    INVALID_EXTEND_FILTER,
    ALL_FILES,
    FAILED_TO_SET_CHARSET_MANUALLY,
    NO_MEMORY,
    CANCEL,
    START_CONVERT,
    INVALID_CHARACTERS, // 30
    WILL_LOST_CHARACTERS,
    NOT_SUPPORT_ENCODING,
    ADD_REDUNDANTLY,
    NO_DETECTED_ENCODING,
    FAILED_TO_WRITE_FILE,
    FILE_SIZE_OUT_OF_LIMIT,
    FAILED_TO_OPEN_FILE,
    FILE_LISTS,
    SET_FILTER_MODE,
    NO_FILTER, // 40
    SMART_FILE_DETECTION,
    USE_FILE_EXTENSION,
    ADD_FILES_OR_FOLDER,
    ADD_FILES,
    ADD_FOLDER,
    SET_OUTPUT,
    OUTPUT_TO_ORIGIN,
    OUTPUT_TO_FOLDER,
    SET_OUTPUT_CHARSET,
    OTHERS, // 50
    CHANGE_LINE_BREAKS,
    CLEAR_LISTS,
    OPEN_WITH_NOTEPAD,
    SPECIFY_ORIGIN_ENCODING,
    REMOVE,
    SELECT_FOLDER,

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

/**
 * 校验语言包。检查是否所有的StringId都有对应的字段。
 * @exception std::runtime_error 校验失败抛出异常。
 */
void CheckLanguagePack(const LanguagePack &langPack);

class LanguageService {
public:
    /**
     * @exception json解析失败抛出异常
     */
    LanguageService(std::function<std::string(void)> fnGetLanguageFromConfig);

    std::string GetCurrentLanguage() const noexcept;

    /**
     * 设置当前语言。会校验语言包，如果校验失败抛出异常。
     * @exception std::runtime_error 校验失败
     */
    void SetCurrentLanguage(const std::string &languageName);

    const std::string &GetUtf8String(StringId id) const noexcept;

    std::wstring GetWString(StringId id) const noexcept;

    std::vector<std::string> GetLanguageArray() const noexcept;

    const std::unordered_map<std::string, std::unique_ptr<LanguagePack>> &GetLanguagesTable() const noexcept;

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