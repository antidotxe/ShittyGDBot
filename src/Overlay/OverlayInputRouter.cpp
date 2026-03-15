#include <GdBot/Overlay/ImGuiOverlayBackend.hpp>
#include <GdBot/Overlay/OverlayInputRouter.hpp>
#include <GdBot/Core/Logging.hpp>

#include <Geode/Geode.hpp>
#include <Geode/cocos/platform/win32/CCEGLView.h>
#include <backends/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

namespace gdbot {

using namespace geode::prelude;

namespace {

bool isMouseMessage(UINT message) {
    switch (message) {
        case WM_MOUSEMOVE:
        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_XBUTTONDBLCLK:
        case WM_MOUSELEAVE:
            return true;
        default:
            return false;
    }
}

bool isKeyboardMessage(UINT message) {
    switch (message) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_CHAR:
        case WM_SYSCHAR:
        case WM_UNICHAR:
            return true;
        default:
            return false;
    }
}

struct WindowSearchState {
    DWORD processId = 0;
    HWND hwnd = nullptr;
};

BOOL CALLBACK findProcessWindow(HWND hwnd, LPARAM lParam) {
    auto& state = *reinterpret_cast<WindowSearchState*>(lParam);

    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);

    if (processId != state.processId) {
        return TRUE;
    }

    if (!IsWindowVisible(hwnd) || GetWindow(hwnd, GW_OWNER) != nullptr) {
        return TRUE;
    }

    state.hwnd = hwnd;
    return FALSE;
}

HWND resolveGameWindow() {
    WindowSearchState state;
    state.processId = GetCurrentProcessId();

    EnumWindows(&findProcessWindow, reinterpret_cast<LPARAM>(&state));
    if (state.hwnd != nullptr) {
        return state.hwnd;
    }

    auto hwnd = GetActiveWindow();
    if (hwnd != nullptr) {
        return hwnd;
    }

    return GetForegroundWindow();
}

}

OverlayInputRouter& OverlayInputRouter::get() {
    static OverlayInputRouter instance;
    return instance;
}

bool OverlayInputRouter::attachIfNeeded() {
    auto resolvedWindow = resolveGameWindow();
    if (resolvedWindow == nullptr) {
        if (!hasLoggedMissingWindow_) {
            logging::warning("GdBot could not resolve a Win32 game window for overlay input");
            hasLoggedMissingWindow_ = true;
        }
        return false;
    }

    if (resolvedWindow == hwnd_ && originalWndProc_ != nullptr) {
        return true;
    }

    detach();

    SetLastError(0);
    auto previousProc = SetWindowLongPtr(
        resolvedWindow,
        GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(&OverlayInputRouter::windowProc)
    );
    if (previousProc == 0 && GetLastError() != 0) {
        logging::warning("GdBot could not subclass the Geometry Dash window for overlay input");
        return false;
    }

    hwnd_ = resolvedWindow;
    originalWndProc_ = reinterpret_cast<WNDPROC>(previousProc);
    hasLoggedMissingWindow_ = false;
    logging::debug(
        "[overlay/input] Attached Win32 input router to window 0x{:X}",
        reinterpret_cast<std::uintptr_t>(hwnd_)
    );
    syncInteractionState(isVisible_);
    return true;
}

void OverlayInputRouter::detach() {
    restoreInteractionState();

    if (hwnd_ != nullptr && originalWndProc_ != nullptr && IsWindow(hwnd_)) {
        SetWindowLongPtr(hwnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalWndProc_));
    }

    if (hwnd_ != nullptr || originalWndProc_ != nullptr) {
        logging::debug("[overlay/input] Detached Win32 input router");
    }
    hwnd_ = nullptr;
    originalWndProc_ = nullptr;
    hasLoggedMissingWindow_ = false;
}

void OverlayInputRouter::syncInteractionState(bool isVisible) {
    auto const previousVisibility = isVisible_;
    isVisible_ = isVisible;

    auto* view = cocos2d::CCEGLView::get();
    if (view == nullptr) {
        return;
    }

    if (isVisible_) {
        if (!hadInteractionState_) {
            previousCursorLocked_ = view->getCursorLocked();
            previousShouldHideCursor_ = view->getShouldHideCursor();
            hadInteractionState_ = true;
        }

        view->toggleLockCursor(false);
        view->showCursor(true);
        if (!previousVisibility) {
            logging::debug("[overlay/input] Enabled overlay input capture");
        }
        return;
    }

    restoreInteractionState();
    if (previousVisibility) {
        logging::debug("[overlay/input] Restored game input capture");
    }
}

HWND OverlayInputRouter::getWindowHandle() const {
    return hwnd_;
}

bool OverlayInputRouter::isAttached() const {
    return hwnd_ != nullptr && originalWndProc_ != nullptr;
}

void OverlayInputRouter::restoreInteractionState() {
    if (!hadInteractionState_) {
        return;
    }

    auto* view = cocos2d::CCEGLView::get();
    if (view != nullptr) {
        view->toggleLockCursor(previousCursorLocked_);
        view->showCursor(!previousShouldHideCursor_);
    }

    ReleaseCapture();
    hadInteractionState_ = false;
}

LRESULT CALLBACK OverlayInputRouter::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    auto& router = get();

    if (router.isVisible_) {
        if (ImGuiOverlayBackend::get().isInitialized()) {
            ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam);
        }

        if (isMouseMessage(message) || isKeyboardMessage(message)) {
            return 0;
        }
    }

    auto result = router.originalWndProc_ == nullptr
        ? DefWindowProc(hwnd, message, wParam, lParam)
        : CallWindowProc(router.originalWndProc_, hwnd, message, wParam, lParam);

    if (message == WM_NCDESTROY) {
        logging::debug("[overlay/input] Observed game window destruction; releasing overlay input router");
        router.restoreInteractionState();
        router.hwnd_ = nullptr;
        router.originalWndProc_ = nullptr;
        router.hasLoggedMissingWindow_ = false;
        ImGuiOverlayBackend::get().shutdown();
    }

    return result;
}

}
