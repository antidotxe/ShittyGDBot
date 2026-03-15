#include <GdBot/Overlay/ImGuiOverlayBackend.hpp>
#include <GdBot/Core/Logging.hpp>

#include <Geode/Geode.hpp>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>

namespace gdbot {

using namespace geode::prelude;

namespace {

constexpr auto kOpenGlShaderVersion = "#version 120";

}

ImGuiOverlayBackend& ImGuiOverlayBackend::get() {
    static ImGuiOverlayBackend instance;
    return instance;
}

bool ImGuiOverlayBackend::ensureInitialized(HWND hwnd) {
    if (hwnd == nullptr) {
        if (!hasLoggedMissingWindow_) {
            logging::warning("GdBot could not initialize the ImGui backend without a game window");
            hasLoggedMissingWindow_ = true;
        }
        return false;
    }

    if (isInitialized_) {
        if (hwnd_ == hwnd) {
            return true;
        }

        logging::info("GdBot detected a window handle change and is rebuilding the ImGui backend");
        shutdown();
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_InitForOpenGL(hwnd)) {
        logging::error("GdBot failed to initialize the ImGui Win32 backend");
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init(kOpenGlShaderVersion)) {
        logging::error("GdBot failed to initialize the ImGui OpenGL3 backend");
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    hwnd_ = hwnd;
    isInitialized_ = true;
    hasLoggedMissingWindow_ = false;
    logging::info("[overlay/backend] Initialized ImGui Win32/OpenGL backend");
    return true;
}

bool ImGuiOverlayBackend::beginFrame() {
    if (!isInitialized_) {
        return false;
    }

    if (ImGui::GetCurrentContext() == nullptr) {
        logging::error("GdBot lost its ImGui context; rebuilding the overlay backend");
        shutdown();
        return false;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    return true;
}

void ImGuiOverlayBackend::endFrame() {
    if (!isInitialized_ || ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiOverlayBackend::shutdown() {
    if (!isInitialized_) {
        hwnd_ = nullptr;
        hasLoggedMissingWindow_ = false;
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();

    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui::DestroyContext();
    }

    hwnd_ = nullptr;
    isInitialized_ = false;
    hasLoggedMissingWindow_ = false;
    logging::debug("[overlay/backend] Shutdown complete");
}

bool ImGuiOverlayBackend::isInitialized() const {
    return isInitialized_;
}

}
