#pragma once

#include <freetype/freetype.h>
#include <fmt/format.h>

#include <stdexcept>
#include <memory>
#include <functional>

template <typename Container = std::unordered_set<FT_Long>>
class FontAnalyzer {
public:
    using InsertFunc = std::function<void(Container &, typename Container::value_type)>;

    FontAnalyzer(InsertFunc insertFunc)
        : library([]() -> std::unique_ptr<FT_LibraryRec_, void (*)(FT_LibraryRec_ *p)> {
              FT_Library rawLibrary;
              FT_Error error = FT_Init_FreeType(&rawLibrary);
              if (error) {
                  throw std::runtime_error(
                      fmt::format("Failed to initialize FreeType library. error code = {}", error));
              }
              return std::unique_ptr<FT_LibraryRec_, void (*)(FT_LibraryRec_ *p)>(rawLibrary, [](FT_LibraryRec_ *lib) {
                  FT_Done_FreeType(lib);
              });
          }()),
          insertFunc(insertFunc) {}

    Container GetUnicodePointRange(const std::string &fontFilePath) const {
        // Load font file
        FT_Face face;
        FT_Error error = FT_New_Face(library.get(), fontFilePath.c_str(), 0, &face);
        if (error == FT_Err_Unknown_File_Format) {
            throw std::runtime_error(
                fmt::format("Font file could be opened and read, but it appears that its font format is unsupported."));
        } else if (error) {
            throw std::runtime_error(fmt::format("Font file could be opened and read, but it appears that its font "
                                                 "format is unsupported. error code = {}",
                                                 error));
        }

        // Select the Unicode character map.
        error = FT_Select_Charmap(face, ft_encoding_unicode);
        if (error) {
            throw std::runtime_error(fmt::format("failed at FT_Select_Charmap. error code = {}", error));
        }

        // Iterate over the character map to find supported Unicode characters.
        FT_ULong charcode = FT_Get_First_Char(face, nullptr);
        FT_ULong prev_charcode = charcode;
        Container seen;

        while (charcode != 0) {
            insertFunc(seen, static_cast<Container::value_type>(charcode));
            charcode = FT_Get_Next_Char(face, charcode, nullptr);
        }

        // Cleanup
        FT_Done_Face(face);
        return seen;
    }

private:
    std::unique_ptr<FT_LibraryRec_, void (*)(FT_LibraryRec_ *p)> library;
    InsertFunc insertFunc;
};
