#pragma once

#include <Core/CharsetCode.h>
#include <Core/LineBreaks.h>
#include <Translator/LanguageService.h>

#include <imgui.h>
#include <fmt/format.h>

class ListView {
public:
    ListView(LanguageService &languageService) noexcept;

    enum class ListViewColumn { INDEX = 0, FILENAME, FILESIZE, ENCODING, LINE_BREAK, TEXT_PIECE };

    struct MyItem {
        int index;
        std::string fileName;
        int fileSize;
        CharsetCode encoding;
        LineBreaks lineBreak;
        std::string textPiece;

        const std::tuple<const int &, const std::string &, const int &, const CharsetCode &, const LineBreaks &,
                         const std::string &>
        AsTuple() const noexcept {
            return std::tie(index, fileName, fileSize, encoding, lineBreak, textPiece);
        }
    };

    void Render();

private:
    LanguageService &languageService;
};