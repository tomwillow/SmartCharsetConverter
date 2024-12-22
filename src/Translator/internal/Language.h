#pragma once

#include "Translator/StringId.h"

// third party
#include <nlohmann/json.hpp>

// standard
#include <map>
#include <functional>

namespace internal {

struct LanguagePack {
    std::string language;
    int langId;
    std::string author;
    std::string version;
    std::string date;
    std::unordered_map<int, std::string> data;

    /**
     * @exception json解析失败抛出异常
     */
    LanguagePack(const std::wstring &filename);

    LanguagePack(int resourceId, const std::wstring &resourceType);

    const std::string &GetString(v0_2::StringId sid) const;

    /**
     * 校验语言包。检查是否所有的StringId都有对应的字段。
     * @exception std::runtime_error 校验失败抛出异常。
     */
    void CheckLanguagePack();
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LanguagePack, language, langId, author, version, date, data)

} // namespace internal