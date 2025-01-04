#pragma once

#include <imgui.h>

#include <future>

void LoadFonts(ImFontAtlas *ioFonts);

class FontLoader {
public:
    FontLoader() {}

    void StartAsyncLoad() {
        assert(!fu.valid());
        fu = std::async(std::launch::async, []() -> std::unique_ptr<ImFontAtlas> {
            std::unique_ptr<ImFontAtlas> ioFonts(IM_NEW(ImFontAtlas));
            LoadFonts(ioFonts.get());
            return ioFonts;
        });
    }

    std::unique_ptr<ImFontAtlas> TryGetFontAtlas() {
        if (!fu.valid()) {
            return nullptr;
        }
        if (fu.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            return fu.get();
        }
        return nullptr;
    }

private:
    std::future<std::unique_ptr<ImFontAtlas>> fu;
};