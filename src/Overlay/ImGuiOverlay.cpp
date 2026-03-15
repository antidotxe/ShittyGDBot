#include <GdBot/Features/Macro/BotController.hpp>
#include <GdBot/Features/Cosmetics/CosmeticUnlockService.hpp>
#include <GdBot/Features/Macro/InputRecorder.hpp>
#include <GdBot/Features/Macro/ReplayController.hpp>
#include <GdBot/Core/Logging.hpp>
#include <GdBot/Overlay/ImGuiOverlay.hpp>
#include <GdBot/Overlay/ImGuiOverlayBackend.hpp>
#include <GdBot/Overlay/OverlayInputRouter.hpp>
#include <GdBot/Overlay/OverlayTheme.hpp>

#include <Geode/Geode.hpp>
#include <imgui.h>

#include <vector>

namespace gdbot {

using namespace geode::prelude;

namespace {

constexpr auto kOverlayToggleKey = cocos2d::enumKeyCodes::KEY_Tab;

ImVec4 getReportColor(CosmeticUnlockState state) {
    switch (state) {
        case CosmeticUnlockState::kSucceeded:
            return ImVec4(0.96F, 0.85F, 0.25F, 1.0F);

        case CosmeticUnlockState::kWarning:
            return ImVec4(1.0F, 0.72F, 0.28F, 1.0F);

        case CosmeticUnlockState::kError:
            return ImVec4(0.95F, 0.34F, 0.34F, 1.0F);

        case CosmeticUnlockState::kIdle:
            return ImGui::GetStyleColorVec4(ImGuiCol_Text);
    }

    return ImGui::GetStyleColorVec4(ImGuiCol_Text);
}

void drawTerminalEntries(
    std::vector<InputLogEntry> const& entries,
    ImU32 markerColor
) {
    if (entries.empty()) {
        ImGui::TextUnformatted("No session activity yet.");
        return;
    }

    auto const shouldStickToBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY();
    for (auto const& entry : entries) {
        if (entry.type == InputLogEntryType::kMarker) {
            ImGui::PushStyleColor(ImGuiCol_Text, markerColor);
        }
        ImGui::TextUnformatted(entry.text.c_str());
        if (entry.type == InputLogEntryType::kMarker) {
            ImGui::PopStyleColor();
        }
    }

    if (shouldStickToBottom) {
        ImGui::SetScrollHereY(1.0F);
    }
}

}

ImGuiOverlay& ImGuiOverlay::get() {
    static ImGuiOverlay instance;
    return instance;
}

void ImGuiOverlay::renderFrame() {
    if (!isVisible_) {
        return;
    }

    auto& inputRouter = OverlayInputRouter::get();
    if (!inputRouter.attachIfNeeded()) {
        return;
    }

    auto& backend = ImGuiOverlayBackend::get();
    if (!backend.ensureInitialized(inputRouter.getWindowHandle())) {
        return;
    }

    if (!backend.beginFrame()) {
        return;
    }

    drawOverlay();

    backend.endFrame();
}

void ImGuiOverlay::shutdown() {
    logging::debug("[overlay] Shutdown requested");
    setVisibility(false);
    ImGuiOverlayBackend::get().shutdown();
    OverlayInputRouter::get().detach();
}

void ImGuiOverlay::toggleVisibility() {
    setVisibility(!isVisible_);
}

void ImGuiOverlay::show() {
    setVisibility(true);
}

void ImGuiOverlay::hide() {
    setVisibility(false);
}

bool ImGuiOverlay::handleKeyboardInput(cocos2d::enumKeyCodes key, bool isKeyDown, bool isKeyRepeat) {
    if (key == kOverlayToggleKey && isKeyDown && !isKeyRepeat) {
        toggleVisibility();
        return true;
    }

    return shouldCaptureKeyboard();
}

bool ImGuiOverlay::shouldCaptureKeyboard() const {
    return isVisible_;
}

void ImGuiOverlay::setVisibility(bool isVisible) {
    if (isVisible_ == isVisible) {
        return;
    }

    isVisible_ = isVisible;
    if (isVisible_) {
        logging::info("[overlay] Opened");
        OverlayInputRouter::get().attachIfNeeded();
        if (BotController::get().isArmed()) {
            logging::debug("[overlay] Disarming playback while the overlay is visible");
            BotController::get().disarm();
        }
    }
    else {
        logging::info("[overlay] Closed");
    }

    OverlayInputRouter::get().syncInteractionState(isVisible_);
}

bool ImGuiOverlay::isVisible() const {
    return isVisible_;
}

void ImGuiOverlay::drawOverlay() {
    auto const& metrics = overlay::getOverlayThemeMetrics();
    auto wasVisible = isVisible_;

    overlay::pushOverlayTheme();
    ImGui::SetNextWindowSize(
        ImVec2(metrics.overlayWidth, metrics.overlayHeight),
        ImGuiCond_FirstUseEver
    );
    ImGui::SetNextWindowBgAlpha(metrics.overlayBackgroundAlpha);

    if (!ImGui::Begin(
            "CysenseOverlay",
            &isVisible_,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
        )) {
        ImGui::End();
        overlay::popOverlayTheme();
        return;
    }

    overlay::drawOverlayHeader("Cysense");
    ImGui::Dummy(ImVec2(0.0F, metrics.bodyTopSpacing));

    if (ImGui::BeginTabBar("CysenseTabs")) {
        if (ImGui::BeginTabItem("Status")) {
            drawStatusTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Cosmetics")) {
            drawCosmeticsTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Terminal")) {
            drawTerminalTab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    overlay::popOverlayTheme();

    if (wasVisible && !isVisible_) {
        OverlayInputRouter::get().syncInteractionState(false);
    }
}

void ImGuiOverlay::drawStatusTab() {
    auto const& metrics = overlay::getOverlayThemeMetrics();
    auto& botController = BotController::get();
    auto& recorder = InputRecorder::get();
    auto& replayController = ReplayController::get();
    auto const& savedRecordingInfo = recorder.getSavedRecordingInfo();
    auto const isRecording = recorder.isRecording();
    auto const recordedInputCount = recorder.getRecordedInputCount();
    auto const hasRecordedInputs = recordedInputCount > 0U;

    ImGui::TextWrapped("Recording captures a macro. Playback uses the saved macro and follows level progress.");
    ImGui::Dummy(ImVec2(0.0F, metrics.tabSpacing));

    if (ImGui::Button(isRecording ? "Stop Recording" : "Start Recording")) {
        if (isRecording) {
            recorder.stopRecording();
        }
        else {
            botController.disarm();
            recorder.startRecording();
            hide();
        }
    }

    ImGui::BeginDisabled(!hasRecordedInputs || isRecording);
    if (ImGui::Button(botController.isArmed() ? "Stop Playback" : "Start Playback")) {
        botController.toggleArmed();
        if (botController.isArmed()) {
            hide();
        }
    }
    ImGui::EndDisabled();

    ImGui::Dummy(ImVec2(0.0F, metrics.tabSpacing));
    ImGui::Text("Saved inputs: %zu", recordedInputCount);
    ImGui::TextWrapped("Saved level: %s", savedRecordingInfo.levelName.c_str());
    ImGui::TextWrapped("%s", isRecording ? "Recording status: active" : "Recording status: idle");
    if (!hasRecordedInputs) {
        ImGui::TextWrapped("No saved macro available yet.");
    }
    ImGui::TextWrapped("%s", botController.getStatusText().c_str());
    ImGui::TextWrapped("%s", replayController.getStatusText().c_str());
    ImGui::Spacing();
    ImGui::Text("Frame rate: %.1f FPS", ImGui::GetIO().Framerate);
}

void ImGuiOverlay::drawCosmeticsTab() {
    auto const& metrics = overlay::getOverlayThemeMetrics();
    auto& unlockService = CosmeticUnlockService::get();

    auto const* updatedReport = &unlockService.getLastReport();
    if (ImGui::Button("Unlock All Cosmetics")) {
        updatedReport = &unlockService.unlockAllCosmetics();
    }

    ImGui::Dummy(ImVec2(0.0F, metrics.tabSpacing));
    ImGui::PushStyleColor(ImGuiCol_Text, getReportColor(updatedReport->state));
    ImGui::TextWrapped("%s", updatedReport->statusText.c_str());
    ImGui::PopStyleColor();
}

void ImGuiOverlay::drawTerminalTab() {
    auto const& metrics = overlay::getOverlayThemeMetrics();
    auto& recorder = InputRecorder::get();
    auto const& entries = recorder.getEntries();
    auto const& session = recorder.getSession();
    auto const& savedRecordingInfo = recorder.getSavedRecordingInfo();

    if (ImGui::Button("Clear")) {
        recorder.clear();
    }

    ImGui::SameLine();
    ImGui::Text(
        "Attempt %d | %zu entries",
        session.attemptIndex,
        recorder.getEntryCount()
    );
    ImGui::TextWrapped("Level: %s", session.levelName.c_str());
    ImGui::TextWrapped("Saved macro: %s", savedRecordingInfo.levelName.c_str());

    ImGui::Dummy(ImVec2(0.0F, metrics.tabSpacing));

    if (ImGui::BeginChild("TerminalLog", ImVec2(0.0F, 0.0F), true, ImGuiWindowFlags_HorizontalScrollbar)) {
        drawTerminalEntries(entries, metrics.terminalMarkerColor);
    }

    ImGui::EndChild();
}

}
