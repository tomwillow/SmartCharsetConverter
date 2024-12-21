#include "Language.h"

#include <Common/CommandLineParser.h>
#include <Common/ResourceLoader.h>

#include <Common/tstring.h>

// standard
#include <cassert>
#include <fstream>
#include <filesystem>

const char DEFAULT_LANGUAGE[] = u8"English";

/**
 * @exception json解析失败抛出异常
 */
LanguagePack LoadLanguageFile(const std::wstring &filename) {
    std::ifstream ifs(to_string(filename));
    if (!ifs) {
        throw std::runtime_error("open file fail: " + to_utf8(filename));
    }

    nlohmann::json j = nlohmann::json::parse(ifs);

    LanguagePack langPack;
    from_json(j, langPack);

    ifs.close();
    return langPack;
}

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

    // check language file
    CheckLanguagePack(*currentLang);
}

std::string LanguageService::GetCurrentLanguage() const noexcept {
    return currentLang->language;
}

const std::string &LanguageService::GetUtf8String(StringId id) const noexcept {
    return currentLang->data.at(id);
}

std::wstring LanguageService::GetWString(StringId id) const noexcept {
    return utf8_to_wstring(GetUtf8String(id));
}

void LanguageService::LoadLanguageNameFromInnerRCFile() noexcept {
    for (auto id : option.resourceIds) {
        std::vector<char> jsonData = LoadCustomFileFromResource(id, option.resourceType);
        jsonData.push_back('\0');

        nlohmann::json j = nlohmann::json::parse(jsonData.data());

        LanguagePack langPack;
        from_json(j, langPack);

#ifndef NDEBUG
        // check file content at Debug
        CheckLanguagePack(langPack);
#endif

        auto langName = langPack.language;
        languages.emplace(langName, std::make_unique<LanguagePack>(std::move(langPack)));
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
        LanguagePack langPack;
        try {
            langPack = LoadLanguageFile(path.path().wstring());
        } catch (const nlohmann::json::exception &err) {
            throw std::runtime_error("failed to load language file from " + path.path().u8string() +
                                     " \r\nReason: " + err.what());
        }

        if (langPack.language.empty()) {
            throw std::runtime_error("failed to load language file from " + path.path().u8string() +
                                     " \r\nReason: \"language\" field is empty");
        }

        auto langName = langPack.language;
        // make external language file override inner language file from the .rc file
        languages[langName] = std::make_unique<LanguagePack>(std::move(langPack));
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

    // check language file
    CheckLanguagePack(*currentLang);
}

std::vector<std::string> LanguageService::GetLanguageArray() const noexcept {
    std::vector<std::string> ret;
    for (auto &pr : languages) {
        ret.push_back(pr.first);
    }
    return ret;
}

LanguageService *&GetLanguageServicePtr() noexcept {
    static LanguageService *ptr = nullptr;
    return ptr;
}

void CheckLanguagePack(const LanguagePack &langPack) {
    for (int i = static_cast<int>(StringId::BEGIN) + 1; i < static_cast<int>(StringId::END); ++i) {
        StringId sid = static_cast<StringId>(i);
        if (langPack.data.find(sid) == langPack.data.end()) {
            throw std::runtime_error("Error at language pack of " + langPack.language +
                                     "\r\ninvalid language pack: lack of id of " + std::to_string(i));
        }
    }
}

void InitLanguageService(LanguageServiceOption option) {
    GetLanguageServicePtr() = new LanguageService(option);
}

void ReleaseLanguageService() noexcept {
    delete GetLanguageServicePtr();
}

LanguageService &GetLanguageService() noexcept {
    assert(GetLanguageServicePtr());
    return *GetLanguageServicePtr();
}