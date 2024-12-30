#include "FontLoader.h"

#include "FontAnalyzer.h"

#include <imgui.h>

#include <string>
#include <vector>

struct FontInfo {
    std::string fontPath;
    int fontSize;
    std::vector<ImWchar> dict;
};

constexpr char windowsFontDir[] = "C:\\Windows\\Fonts\\";

void LoadFonts(ImGuiIO &io) {

    std::vector<FontInfo> fontInfos{
        FontInfo{std::string(windowsFontDir) + "segoeui.ttf", 20, std::vector<ImWchar>{}},
        FontInfo{std::string(windowsFontDir) + "msyh.ttc", 20, std::vector<ImWchar>{}},
        FontInfo{std::string(windowsFontDir) + "simsun.ttc", 20, std::vector<ImWchar>{}},
    };

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use
    // ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your
    // application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double
    // backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See
    // Makefile.emscripten for details.
    // io.Fonts->AddFontDefault();

    ImFontConfig fontConfig;
    bool loadedTheFirst = false;

    for (std::size_t i = 0; i < fontInfos.size(); ++i) {
        if (loadedTheFirst) {
            fontConfig.MergeMode = true;
        }
        // FontAnalyzer<std::unordered_set<uint32_t>> fontAnalyzer([](std::unordered_set<uint32_t> &c, uint32_t val) {
        //     c.insert(val);
        // });
        FontAnalyzer<std::vector<ImWchar>> fontAnalyzer([](std::vector<ImWchar> &c, ImWchar val) {
            c.push_back(val);
        });
        auto ret = fontAnalyzer.GetUnicodePointRange(fontInfos[i].fontPath);
        ret.push_back(0);
        fontInfos[i].dict = std::move(ret);

        ImFont *font =
            io.Fonts->AddFontFromFileTTF(fontInfos[i].fontPath.c_str(), static_cast<float>(fontInfos[i].fontSize),
                                         &fontConfig, fontInfos[i].dict.data());
        assert(font);
        loadedTheFirst = true;
    }

    io.Fonts->Build();
    for (auto font : io.Fonts->Fonts) {
        assert(font->IsLoaded());
    }
}