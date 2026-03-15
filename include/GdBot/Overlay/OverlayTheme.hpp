#pragma once

#include <imgui.h>

namespace gdbot::overlay {

struct OverlayThemeMetrics {
    float overlayWidth = 430.0F;
    float overlayHeight = 320.0F;
    float overlayBackgroundAlpha = 0.92F;
    float headerTextOffsetX = 16.0F;
    float headerTextOffsetY = 12.0F;
    float headerLineY = 34.0F;
    float headerLineThickness = 2.0F;
    float bodyTopSpacing = 28.0F;
    float tabSpacing = 10.0F;
    ImU32 headerColor = 0;
    ImU32 terminalMarkerColor = 0;
};

[[nodiscard]] OverlayThemeMetrics const& getOverlayThemeMetrics();
void pushOverlayTheme();
void popOverlayTheme();
void drawOverlayHeader(char const* title);

}
