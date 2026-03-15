#include <GdBot/Features/Macro/ReplayController.hpp>

#include <GdBot/Features/Macro/BotController.hpp>
#include <GdBot/Features/Macro/InputRecorder.hpp>
#include <GdBot/Core/Logging.hpp>

#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>

#include <algorithm>
#include <array>
#include <fmt/format.h>
#include <sstream>
#include <vector>

namespace gdbot {

namespace {

constexpr std::size_t kMaxDetailedReplayLogs = 12;

bool hasActiveCheckpoint(PlayLayer* playLayer) {
    return playLayer != nullptr &&
        playLayer->m_checkpointArray != nullptr &&
        playLayer->m_checkpointArray->count() > 0U;
}

bool shouldPreserveReplayProgress(PlayLayer* playLayer, std::size_t cursor) {
    return playLayer != nullptr &&
        BotController::get().isArmed() &&
        cursor > 0U &&
        hasActiveCheckpoint(playLayer);
}

std::string describePlaybackWindow(std::vector<RecordedInputEvent> const& events) {
    if (events.empty()) {
        return "no saved macro";
    }

    auto const& firstEvent = events.front();
    auto const& lastEvent = events.back();
    return fmt::format(
        "{} inputs, progress {} -> {}, duration {:.3f}s",
        events.size(),
        firstEvent.progressStep,
        lastEvent.progressStep,
        lastEvent.timeSeconds
    );
}

}

ReplayController& ReplayController::get() {
    static ReplayController instance;
    return instance;
}

void ReplayController::beginLevelSession() {
    clearState();
    isSessionActive_ = true;
    isTimelineDirty_ = true;
    logging::info("Replay session started");
}

void ReplayController::endLevelSession() {
    logging::info("Replay session ended");
    clearState();
}

void ReplayController::markRecordingDirty() {
    isTimelineDirty_ = true;
}

void ReplayController::handleTimelineReset(PlayLayer* playLayer) {
    if (playLayer == nullptr) {
        logging::warning("Replay reset requested with no active PlayLayer");
        clearState();
        return;
    }

    auto const progressStep = resolvePlaybackProgressStep(playLayer);

    if (shouldPreserveReplayProgress(playLayer, cursor_)) {
        releaseHeldButtons(playLayer);
        hasObservedProgressStep_ = true;
        lastObservedProgressStep_ = progressStep;
        logging::debug(
            "Replay preserved progress across reset at step {}, cursor={}",
            progressStep,
            cursor_
        );
        return;
    }

    logging::info("Replay timeline reset at step {}", progressStep);
    rebaseToProgress(playLayer, progressStep, true);
}

void ReplayController::update(GJBaseGameLayer* gameLayer) {
    if (gameLayer == nullptr || !isSessionActive_) {
        return;
    }

    auto const isArmed = BotController::get().isArmed();
    auto const currentProgressStep = resolvePlaybackProgressStep(gameLayer);
    if (isArmed != lastArmedState_) {
        handleArmedStateChange(gameLayer, isArmed, currentProgressStep);
    }

    if (!isArmed) {
        return;
    }

    auto const& recorder = InputRecorder::get();
    if (isTimelineDirty_ || sourceEventCount_ != recorder.getRecordedInputCount()) {
        rebuildTimeline(currentProgressStep);
    }

    if (events_.empty()) {
        hasObservedProgressStep_ = true;
        lastObservedProgressStep_ = currentProgressStep;
        return;
    }

    auto* playLayer = static_cast<PlayLayer*>(gameLayer);
    if (hasObservedProgressStep_ && currentProgressStep < lastObservedProgressStep_) {
        if (shouldPreserveReplayProgress(playLayer, cursor_)) {
            releaseHeldButtons(gameLayer);
            lastObservedProgressStep_ = currentProgressStep;
            logging::debug(
                "Replay preserved cursor after backward jump to step {}, cursor={}",
                currentProgressStep,
                cursor_
            );
            return;
        }

        logging::debug(
            "Replay detected backward progress jump from step {} to step {}",
            lastObservedProgressStep_,
            currentProgressStep
        );
        rebaseToProgress(gameLayer, currentProgressStep, true);
    }

    while (cursor_ < events_.size() && events_[cursor_].progressStep <= currentProgressStep) {
        applyEvent(gameLayer, events_[cursor_]);
        ++cursor_;
    }

    hasObservedProgressStep_ = true;
    lastObservedProgressStep_ = currentProgressStep;
}

void ReplayController::handleArmedStateChange(
    GJBaseGameLayer* gameLayer,
    bool isArmed,
    int currentProgressStep
) {
    if (isArmed) {
        logging::info(
            "Replay armed at step {} with {}",
            currentProgressStep,
            describePlaybackWindow(InputRecorder::get().getRecordedInputs())
        );
    }
    else {
        logging::info("Replay disarmed at step {}", currentProgressStep);
    }
    lastArmedState_ = isArmed;

    if (isArmed) {
        return;
    }

    rebaseToProgress(gameLayer, currentProgressStep, true);
}

bool ReplayController::isInjectingInput() const {
    return isInjectingInput_;
}

std::string ReplayController::getStatusText() const {
    if (!isSessionActive_) {
        return "Replay: no active level";
    }

    if (events_.empty()) {
        return "Replay: no saved macro loaded";
    }

    std::ostringstream stream;
    stream << "Replay: " << events_.size() << " inputs loaded";

    if (BotController::get().isArmed()) {
        stream << " | cursor " << cursor_ << "/" << events_.size();
    }
    else {
        stream << " | idle";
    }

    return stream.str();
}

void ReplayController::rebuildTimeline(int currentProgressStep) {
    auto const& recordedInputs = InputRecorder::get().getRecordedInputs();

    events_.clear();
    events_.reserve(recordedInputs.size());
    for (auto const& input : recordedInputs) {
        events_.push_back({input.progressStep, input.button, input.edge, input.isPlayer2});
    }

    sourceEventCount_ = recordedInputs.size();
    isTimelineDirty_ = false;
    cursor_ = findCursorForProgress(currentProgressStep, events_);
    heldButtons_.fill(false);
    hasObservedProgressStep_ = true;
    lastObservedProgressStep_ = currentProgressStep;
    loggedReplayEventCount_ = 0;
    skippedDeadEventCount_ = 0;

    logging::debug(
        "Replay timeline rebuilt with {} events at step {}, cursor={}",
        events_.size(),
        currentProgressStep,
        cursor_
    );
}

void ReplayController::releaseHeldButtons(GJBaseGameLayer* gameLayer) {
    if (gameLayer == nullptr) {
        heldButtons_.fill(false);
        return;
    }

    for (auto playerIndex = 0U; playerIndex < 2; ++playerIndex) {
        for (auto buttonIndex = 0U; buttonIndex < 3; ++buttonIndex) {
            auto const heldIndex = playerIndex * 3 + buttonIndex;
            if (!heldButtons_[heldIndex]) {
                continue;
            }

            simulateInput(
                gameLayer,
                static_cast<PlayerButton>(buttonIndex),
                InputEdge::kRelease,
                playerIndex == 1U
            );
            heldButtons_[heldIndex] = false;
        }
    }
}

void ReplayController::clearState() {
    events_.clear();
    cursor_ = 0;
    sourceEventCount_ = 0;
    lastObservedProgressStep_ = 0;
    hasObservedProgressStep_ = false;
    isInjectingInput_ = false;
    isTimelineDirty_ = true;
    isSessionActive_ = false;
    lastArmedState_ = false;
    heldButtons_.fill(false);
    loggedReplayEventCount_ = 0;
    skippedDeadEventCount_ = 0;
}

void ReplayController::rebaseToProgress(
    GJBaseGameLayer* gameLayer,
    int currentProgressStep,
    bool releaseHeldButtonsFirst
) {
    auto const hadObservedProgress = hasObservedProgressStep_;
    auto const previousCursor = cursor_;
    auto const previousProgressStep = lastObservedProgressStep_;

    if (releaseHeldButtonsFirst) {
        releaseHeldButtons(gameLayer);
    }

    cursor_ = findCursorForProgress(currentProgressStep, events_);
    hasObservedProgressStep_ = true;
    lastObservedProgressStep_ = currentProgressStep;

    if (!hadObservedProgress || cursor_ != previousCursor || previousProgressStep != currentProgressStep) {
        logging::debug("Replay rebased to step {}, cursor={}", currentProgressStep, cursor_);
    }
}

void ReplayController::applyEvent(GJBaseGameLayer* gameLayer, ReplayEvent const& event) {
    auto* player = resolvePlayer(gameLayer, event.isPlayer2);
    if (gameLayer == nullptr || player == nullptr) {
        if (skippedDeadEventCount_ < kMaxDetailedReplayLogs) {
            logging::warning(
                "Replay skipped {} {} for {} at step {} because player state was unavailable",
                toString(event.edge),
                static_cast<int>(event.button),
                event.isPlayer2 ? "P2" : "P1",
                event.progressStep
            );
        }
        ++skippedDeadEventCount_;
        return;
    }

    auto const buttonIndex = resolveButtonIndex(event.button, event.isPlayer2);
    simulateInput(gameLayer, event.button, event.edge, event.isPlayer2);
    heldButtons_[buttonIndex] = isPressedEdge(event.edge);

    if (loggedReplayEventCount_ < kMaxDetailedReplayLogs) {
        logging::debug(
            "Replay event {}: {} button {} for {} at step {}",
            loggedReplayEventCount_ + 1,
            toString(event.edge),
            static_cast<int>(event.button),
            event.isPlayer2 ? "P2" : "P1",
            event.progressStep
        );
    }
    else if (loggedReplayEventCount_ == kMaxDetailedReplayLogs) {
        logging::debug("Replay event logging suppressed after {} events", kMaxDetailedReplayLogs);
    }
    ++loggedReplayEventCount_;
}

void ReplayController::simulateInput(
    GJBaseGameLayer* gameLayer,
    PlayerButton button,
    InputEdge edge,
    bool isPlayer2
) {
    if (gameLayer == nullptr || gameLayer->m_player1 == nullptr) {
        return;
    }

    auto player2 = isPlayer2;
    if (auto* gameManager = GameManager::get()) {
        player2 = gameManager->getGameVariable(GameVar::Flip2PlayerControls) ? !player2 : player2;
    }

    auto const performButton = isPressedEdge(edge)
        ? &PlayerObject::pushButton
        : &PlayerObject::releaseButton;

    isInjectingInput_ = true;

    if (gameLayer->m_levelSettings != nullptr &&
        gameLayer->m_levelSettings->m_twoPlayerMode &&
        gameLayer->m_gameState.m_isDualMode) {
        auto* player = player2 ? gameLayer->m_player2 : gameLayer->m_player1;
        if (player != nullptr) {
            (player->*performButton)(button);
        }
    }
    else {
        (gameLayer->m_player1->*performButton)(button);
        if (gameLayer->m_gameState.m_isDualMode && gameLayer->m_player2 != nullptr) {
            (gameLayer->m_player2->*performButton)(button);
        }
    }

    if (gameLayer->m_effectManager != nullptr) {
        gameLayer->m_effectManager->playerButton(isPressedEdge(edge), !player2);
    }

    if (isPressedEdge(edge)) {
        ++gameLayer->m_clicks;
        if (button == PlayerButton::Jump) {
            gameLayer->m_jumping = true;
        }
    }

    isInjectingInput_ = false;
}

std::size_t ReplayController::resolveButtonIndex(PlayerButton button, bool isPlayer2) {
    return static_cast<std::size_t>(button) + (isPlayer2 ? 3U : 0U);
}

std::size_t ReplayController::findCursorForProgress(
    int currentProgressStep,
    std::vector<ReplayEvent> const& events
) {
    auto const firstFutureEvent = std::lower_bound(
        events.begin(),
        events.end(),
        currentProgressStep,
        [](ReplayEvent const& event, int progressStep) {
            return event.progressStep < progressStep;
        }
    );

    return static_cast<std::size_t>(std::distance(events.begin(), firstFutureEvent));
}

int ReplayController::resolvePlaybackProgressStep(GJBaseGameLayer* gameLayer) {
    if (gameLayer == nullptr) {
        return 0;
    }

    return static_cast<int>(gameLayer->m_gameState.m_currentProgress / kMacroProgressDivisor);
}

PlayerObject* ReplayController::resolvePlayer(GJBaseGameLayer* gameLayer, bool isPlayer2) {
    if (gameLayer == nullptr) {
        return nullptr;
    }

    return isPlayer2 ? gameLayer->m_player2 : gameLayer->m_player1;
}

}
