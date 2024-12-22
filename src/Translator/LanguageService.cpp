#include "LanguageService.h"

#include <Common/tstring.h>
#include <Common/ResourceLoader.h>
#include <Common/CommandLineParser.h>

const char DEFAULT_LANGUAGE[] = u8"English";

LanguageService::LanguageService(LanguageServiceOption option) : option(option) {
    /*
        加载流程：
        先从内置的json语言文件加载。
        然后加载外置的json语言文件。如果和内置的重复，那么覆盖内置的。

        从配置读取当前语言，如果没有设置，
        那么读取系统语言。如果系统语言没有对应的语言包，那么加载英语。
    */
    LoadLanguageNameFromInnerRCFile();

    LoadLanguageNameFromDir("lang");

    std::string lang = option.fnGetLanguageFromConfig();
    if (lang.empty()) {
        LANGID langId = GetUserDefaultLangID();

        lang = GetLanguageNameByLangIdFromLoadedLanguages(langId);

        if (lang.empty()) {
            lang = DEFAULT_LANGUAGE;
        }
    }

    if (languages.find(lang) == languages.end()) {
        throw std::runtime_error(
            "language json file is lost. language name = " + lang +
            "\r\n\r\nTip: Remove the configuration json file could make program load default language.");
    }
    currentLang = languages[lang].get();
}

std::string LanguageService::GetCurrentLanguage() const noexcept {
    return currentLang->language;
}

const std::string &LanguageService::GetUtf8String(v0_2::StringId id) const noexcept {
    return currentLang->GetString(id);
}

std::wstring LanguageService::GetWString(v0_2::StringId id) const noexcept {
    return utf8_to_wstring(GetUtf8String(id));
}

void LanguageService::LoadLanguageNameFromInnerRCFile() noexcept {
    for (auto id : option.resourceIds) {
        internal::LanguagePack langPack(id, option.resourceType);

        auto langName = langPack.language;
        languages.emplace(langName, std::make_unique<internal::LanguagePack>(std::move(langPack)));
    }
}

void LanguageService::LoadLanguageNameFromDir(const std::string &dir) {
    // 得到命令行参数
    const std::vector<std::wstring> args = GetCommandLineArgs();
    std::wstring selfPath = args[0];
    std::filesystem::path exeDir = std::filesystem::u8path(to_utf8(selfPath)).parent_path();

    std::filesystem::path langDir = exeDir / dir;
    if (!std::filesystem::is_directory(langDir)) {
        return;
    }

    for (auto path : std::filesystem::directory_iterator(exeDir / dir)) {
        std::unique_ptr<internal::LanguagePack> langPack;
        try {
            langPack = std::make_unique<internal::LanguagePack>(path.path().wstring());
        } catch (const nlohmann::json::exception &err) {
            throw std::runtime_error("failed to load language file from " + path.path().u8string() +
                                     " \r\nReason: " + err.what());
        }

        if (langPack->language.empty()) {
            throw std::runtime_error("failed to load language file from " + path.path().u8string() +
                                     " \r\nReason: \"language\" field is empty");
        }

        auto langName = langPack->language;
        // make external language file override inner language file from the .rc file
        languages[langName] = std::move(langPack);
    }
}

bool LanguageService::HasLanguagePack(const std::string &lang) const noexcept {
    return languages.find(lang) != languages.end();
}

std::string LanguageService::GetLanguageNameByLangIdFromLoadedLanguages(int langId) const noexcept {
    for (auto &pr : languages) {
        if (pr.second->langId == langId) {
            return pr.first;
        }
    }
    return "";
}

void LanguageService::SetCurrentLanguage(const std::string &languageName) {
    currentLang = languages.at(languageName).get();
}

std::vector<std::string> LanguageService::GetLanguageArray() const noexcept {
    std::vector<std::string> ret;
    for (auto &pr : languages) {
        ret.push_back(pr.first);
    }
    return ret;
}

std::string LanguageService::MessageIdToString(MessageId mid) const noexcept {
    return GetUtf8String(static_cast<v0_2::StringId>(mid));
}
