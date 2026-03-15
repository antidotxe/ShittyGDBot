#pragma once

#include <Geode/Enums.hpp>
#include <GdBot/Features/Macro/InputTypes.hpp>

#include <array>
#include <string>
#include <vector>

class GJBaseGameLayer;
class PlayLayer;

namespace gdbot {

class ReplayController {
public:
    static ReplayController& get();

    ReplayController(ReplayController const&) = delete;
    ReplayController& operator=(ReplayController const&) = delete;

    void beginLevelSession();
    void endLevelSession();
    void markRecordingDirty();
    void handleTimelineReset(PlayLayer* playLayer);
    void update(GJBaseGameLayer* gameLayer);

    [[nodiscard]] bool isInjectingInput() const;
    [[nodiscard]] std::string getStatusText() const;

private:
    struct ReplayEvent {
        int progressStep = 0;
        PlayerButton button = PlayerButton::Jump;
        InputEdge edge = InputEdge::kPress;
        bool isPlayer2 = false;
    };

    ReplayController() = default;

    void rebuildTimeline(int currentProgressStep);
    void handleArmedStateChange(GJBaseGameLayer* gameLayer, bool isArmed, int currentProgressStep);
    void releaseHeldButtons(GJBaseGameLayer* gameLayer);
    void clearState();
    void rebaseToProgress(GJBaseGameLayer* gameLayer, int currentProgressStep, bool releaseHeldButtonsFirst);
    void applyEvent(GJBaseGameLayer* gameLayer, ReplayEvent const& event);
    void simulateInput(GJBaseGameLayer* gameLayer, PlayerButton button, InputEdge edge, bool isPlayer2);

    [[nodiscard]] static std::size_t resolveButtonIndex(PlayerButton button, bool isPlayer2);
    [[nodiscard]] static std::size_t findCursorForProgress(int currentProgressStep, std::vector<ReplayEvent> const& events);
    [[nodiscard]] static int resolvePlaybackProgressStep(GJBaseGameLayer* gameLayer);
    [[nodiscard]] static class PlayerObject* resolvePlayer(GJBaseGameLayer* gameLayer, bool isPlayer2);

    std::vector<ReplayEvent> events_;
    std::size_t cursor_ = 0;
    std::size_t sourceEventCount_ = 0;
    int lastObservedProgressStep_ = 0;
    bool hasObservedProgressStep_ = false;
    bool isInjectingInput_ = false;
    bool isTimelineDirty_ = true;
    bool isSessionActive_ = false;
    bool lastArmedState_ = false;
    std::array<bool, 6> heldButtons_{};
    std::size_t loggedReplayEventCount_ = 0;
    std::size_t skippedDeadEventCount_ = 0;
};

}
