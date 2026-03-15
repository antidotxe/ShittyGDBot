#pragma once

namespace gdbot {

enum class InputEdge {
    kPress,
    kRelease,
};

constexpr auto kMacroProgressDivisor = 2;

[[nodiscard]] constexpr bool isPressedEdge(InputEdge edge) {
    return edge == InputEdge::kPress;
}

[[nodiscard]] constexpr char const* toString(InputEdge edge) {
    return isPressedEdge(edge) ? "press" : "release";
}

}
