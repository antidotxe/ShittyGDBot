#pragma once

#include <windows.h>

namespace gdbot {

class OverlayInputRouter {
public:
    static OverlayInputRouter& get();

    OverlayInputRouter(OverlayInputRouter const&) = delete;
    OverlayInputRouter& operator=(OverlayInputRouter const&) = delete;

    bool attachIfNeeded();
    void detach();
    void syncInteractionState(bool isVisible);

    [[nodiscard]] HWND getWindowHandle() const;
    [[nodiscard]] bool isAttached() const;

private:
    OverlayInputRouter() = default;

    void restoreInteractionState();
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND hwnd_ = nullptr;
    WNDPROC originalWndProc_ = nullptr;
    bool isVisible_ = false;
    bool hadInteractionState_ = false;
    bool previousCursorLocked_ = false;
    bool previousShouldHideCursor_ = false;
    bool hasLoggedMissingWindow_ = false;
};

}
