#pragma once

#include "StringId.h"
#include "internal/Language.h"

#include <functional>
#include <vector>

struct LanguageServiceOption {
    // specify the language at starting.
    // the content should be equal of "language" field data of one of language json files.
    std::string languageName;
    std::wstring resourceType;
    std::vector<int> resourceIds;
};

/*
    多语言支持

    加载流程：
    先从内置的json语言文件加载。
    然后加载外置的json语言文件。如果和内置的重复，那么覆盖内置的。

    等待配置读取当前语言，如果没有设置，
    那么读取系统语言。如果系统语言没有对应的语言包，那么加载英语。
*/
class LanguageService : public TranslatorBase {
public:
    /**
     * @exception json解析失败抛出异常
     */
    LanguageService(LanguageServiceOption option);

    std::string GetCurrentLanguage() const noexcept;

    /**
     * 设置当前语言。会校验语言包，如果校验失败抛出异常。
     * @exception std::runtime_error 校验失败
     */
    void SetCurrentLanguage(const std::string &languageName);

    const std::string &GetUtf8String(v0_2::StringId id) const noexcept;

    std::wstring GetWString(v0_2::StringId id) const noexcept;

    std::vector<std::string> GetLanguageArray() const noexcept;

    const std::unordered_map<std::string, std::unique_ptr<internal::LanguagePack>> &GetLanguagesTable() const noexcept;

    virtual std::string MessageIdToString(MessageId mid) const noexcept;

private:
    LanguageServiceOption option;
    std::vector<std::string> avaliableLanguages;
    internal::LanguagePack *currentLang;
    std::map<std::string, std::unique_ptr<internal::LanguagePack>> languages;

    void LoadLanguageNameFromInnerRCFile() noexcept;

    void LoadLanguageNameFromDir(const std::filesystem::path &dir);

    bool HasLanguagePack(const std::string &lang) const noexcept;

    std::string GetLanguageNameByLangIdFromLoadedLanguages(int langId) const noexcept;
};
