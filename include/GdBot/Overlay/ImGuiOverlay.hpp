#pragma once

#include <Geode/Enums.hpp>

namespace gdbot {

class ImGuiOverlay {
public:
    static ImGuiOverlay& get();

    ImGuiOverlay(ImGuiOverlay const&) = delete;
    ImGuiOverlay& operator=(ImGuiOverlay const&) = delete;

    void renderFrame();
    void shutdown();
    void toggleVisibility();
    void show();
    void hide();
    [[nodiscard]] bool handleKeyboardInput(cocos2d::enumKeyCodes key, bool isKeyDown, bool isKeyRepeat);
    [[nodiscard]] bool shouldCaptureKeyboard() const;

    [[nodiscard]] bool isVisible() const;

private:
    ImGuiOverlay() = default;

    void setVisibility(bool isVisible);
    void drawOverlay();
    void drawStatusTab();
    void drawCosmeticsTab();
    void drawTerminalTab();

    bool isVisible_ = false;
};

}
