#include "Language.h"
#include "Translator/StringId.h"

#include <Common/CommandLineParser.h>
#include <Common/ResourceLoader.h>

#include <Common/tstring.h>

// standard
#include <cassert>
#include <stdexcept>
#include <fstream>
#include <filesystem>

namespace internal {

LanguagePack::LanguagePack(const std::wstring &filename) {
    std::ifstream ifs(to_string(filename));
    if (!ifs) {
        throw std::runtime_error("open file fail: " + to_utf8(filename));
    }

    nlohmann::json j = nlohmann::json::parse(ifs);
    from_json(j, *this);

    ifs.close();

#ifndef NDEBUG
    // check file content at Debug
    CheckLanguagePack();
#endif
}

LanguagePack::LanguagePack(int resourceId, const std::wstring &resourceType) {

    std::vector<char> jsonData = LoadCustomFileFromResource(resourceId, resourceType);
    jsonData.push_back('\0');

    nlohmann::json j = nlohmann::json::parse(jsonData.data());

    from_json(j, *this);

#ifndef NDEBUG
    // check file content at Debug
    CheckLanguagePack();
#endif
}

const std::string &LanguagePack::GetString(v0_2::StringId sid) const {
    return data.at(static_cast<int>(sid));
}

void LanguagePack::CheckLanguagePack() {
    if (version != StringIdVersion) {
        throw std::runtime_error("unsupported language file version: " + version);
    }
    for (auto [begin, end] : v0_2::STRING_ID_RANGES) {
        for (int i = begin + 1; i < end; ++i) {
            v0_2::StringId sid = static_cast<v0_2::StringId>(i);
            bool found = data.find(static_cast<int>(sid)) != data.end();
            if (!found) {
                throw std::runtime_error("Error at language pack of " + language +
                                         "\r\ninvalid language pack: lack of id of " + std::to_string(i));
            }
        }
    }
}

} // namespace internal