#include <GdBot/Overlay/OverlayTheme.hpp>

namespace gdbot::overlay {

namespace {

constexpr auto kOverlayThemeColorCount = 23;

OverlayThemeMetrics const kThemeMetrics{
    .overlayWidth = 430.0F,
    .overlayHeight = 320.0F,
    .overlayBackgroundAlpha = 0.92F,
    .headerTextOffsetX = 16.0F,
    .headerTextOffsetY = 12.0F,
    .headerLineY = 34.0F,
    .headerLineThickness = 2.0F,
    .bodyTopSpacing = 28.0F,
    .tabSpacing = 10.0F,
    .headerColor = IM_COL32(245, 205, 66, 255),
    .terminalMarkerColor = IM_COL32(255, 221, 107, 255),
};

}

OverlayThemeMetrics const& getOverlayThemeMetrics() {
    return kThemeMetrics;
}

void pushOverlayTheme() {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.96F, 0.93F, 0.82F, 1.0F));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.96F, 0.80F, 0.26F, 0.55F));
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.96F, 0.80F, 0.26F, 0.75F));
    ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(1.0F, 0.86F, 0.39F, 0.9F));
    ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(0.90F, 0.72F, 0.16F, 1.0F));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.27F, 0.22F, 0.07F, 0.9F));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.34F, 0.28F, 0.08F, 0.95F));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.43F, 0.34F, 0.08F, 1.0F));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.27F, 0.22F, 0.07F, 0.9F));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.34F, 0.28F, 0.08F, 0.95F));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.43F, 0.34F, 0.08F, 1.0F));
    ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.23F, 0.19F, 0.07F, 0.9F));
    ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.37F, 0.30F, 0.09F, 0.98F));
    ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.45F, 0.35F, 0.09F, 1.0F));
    ImGui::PushStyleColor(ImGuiCol_TabSelected, ImVec4(0.45F, 0.35F, 0.09F, 1.0F));
    ImGui::PushStyleColor(ImGuiCol_TabSelectedOverline, ImVec4(0.96F, 0.80F, 0.26F, 1.0F));
    ImGui::PushStyleColor(ImGuiCol_TabDimmed, ImVec4(0.20F, 0.16F, 0.06F, 0.9F));
    ImGui::PushStyleColor(ImGuiCol_TabDimmedSelected, ImVec4(0.32F, 0.26F, 0.08F, 0.95F));
    ImGui::PushStyleColor(ImGuiCol_TabDimmedSelectedOverline, ImVec4(0.96F, 0.80F, 0.26F, 0.85F));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.17F, 0.14F, 0.05F, 0.85F));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25F, 0.20F, 0.06F, 0.95F));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.34F, 0.27F, 0.07F, 1.0F));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.07F, 0.06F, 0.03F, 0.55F));
}

void popOverlayTheme() {
    ImGui::PopStyleColor(kOverlayThemeColorCount);
}

void drawOverlayHeader(char const* title) {
    auto const& metrics = getOverlayThemeMetrics();
    auto* drawList = ImGui::GetWindowDrawList();
    auto windowPos = ImGui::GetWindowPos();
    auto windowSize = ImGui::GetWindowSize();

    drawList->AddText(
        ImVec2(windowPos.x + metrics.headerTextOffsetX, windowPos.y + metrics.headerTextOffsetY),
        metrics.headerColor,
        title
    );
    drawList->AddLine(
        ImVec2(windowPos.x + metrics.headerTextOffsetX, windowPos.y + metrics.headerLineY),
        ImVec2(windowPos.x + windowSize.x - metrics.headerTextOffsetX, windowPos.y + metrics.headerLineY),
        metrics.headerColor,
        metrics.headerLineThickness
    );
}

}
