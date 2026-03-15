#pragma once

#include <windows.h>

namespace gdbot {

class ImGuiOverlayBackend {
public:
    static ImGuiOverlayBackend& get();

    ImGuiOverlayBackend(ImGuiOverlayBackend const&) = delete;
    ImGuiOverlayBackend& operator=(ImGuiOverlayBackend const&) = delete;

    bool ensureInitialized(HWND hwnd);
    bool beginFrame();
    void endFrame();
    void shutdown();

    [[nodiscard]] bool isInitialized() const;

private:
    ImGuiOverlayBackend() = default;

    HWND hwnd_ = nullptr;
    bool isInitialized_ = false;
    bool hasLoggedMissingWindow_ = false;
};

}
