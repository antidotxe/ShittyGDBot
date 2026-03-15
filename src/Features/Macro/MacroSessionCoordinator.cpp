#include <GdBot/Features/Macro/MacroSessionCoordinator.hpp>

#include <GdBot/Features/Macro/InputRecorder.hpp>
#include <GdBot/Features/Macro/ReplayController.hpp>
#include <GdBot/Core/Logging.hpp>

#include <Geode/binding/CheckpointObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>

namespace gdbot {

namespace {

InputRecorder& recorder() {
    return InputRecorder::get();
}

ReplayController& replay() {
    return ReplayController::get();
}

std::string resolveLevelName(GJGameLevel* level) {
    if (level == nullptr) {
        return "No active level";
    }

    auto levelName = std::string(level->m_levelName);
    if (levelName.empty()) {
        return "Unnamed level";
    }

    return levelName;
}

}

MacroSessionCoordinator& MacroSessionCoordinator::get() {
    static MacroSessionCoordinator instance;
    return instance;
}

void MacroSessionCoordinator::didInitializeLevel(GJGameLevel* level) {
    logging::info(
        "[macro/session] Initializing level session for {}",
        resolveLevelName(level)
    );
    recorder().beginLevelSession(level);
    replay().beginLevelSession();
}

void MacroSessionCoordinator::didProcessQueuedButtons(GJBaseGameLayer* gameLayer) {
    replay().update(gameLayer);
}

void MacroSessionCoordinator::didResetLevel(PlayLayer* playLayer) {
    if (playLayer == nullptr) {
        logging::warning("[macro/session] Received reset event without an active PlayLayer");
        return;
    }

    auto const checkpointCount =
        playLayer->m_checkpointArray == nullptr ? 0U : playLayer->m_checkpointArray->count();
    logging::debug("[macro/session] Level reset observed with {} checkpoints", checkpointCount);

    if (playLayer->m_checkpointArray == nullptr || playLayer->m_checkpointArray->count() == 0U) {
        recorder().clearActiveRecording();
    }

    recorder().markAttemptReset();
    replay().handleTimelineReset(playLayer);
}

void MacroSessionCoordinator::willLoadCheckpoint(CheckpointObject* checkpoint) {
    if (checkpoint != nullptr) {
        logging::debug("[macro/session] Loading checkpoint at progress step {}", static_cast<int>(
            checkpoint->m_gameState.m_currentProgress / kMacroProgressDivisor
        ));
    }
    recorder().trimRecordingToCheckpoint(checkpoint);
}

void MacroSessionCoordinator::didStoreCheckpoint() {
    logging::debug("[macro/session] Stored a practice checkpoint");
    recorder().markCheckpointPlaced();
}

void MacroSessionCoordinator::willDestroyPlayer(PlayerObject* player) {
    logging::debug(
        "[macro/session] Forwarding {} death event",
        player != nullptr && player->m_isSecondPlayer ? "P2" : "P1"
    );
    recorder().markPlayerDied(player != nullptr && player->m_isSecondPlayer);
}

void MacroSessionCoordinator::willCompleteLevel(PlayLayer* playLayer) {
    logging::info("[macro/session] Level completion observed");
    recorder().markLevelComplete();
    replay().handleTimelineReset(playLayer);
}

void MacroSessionCoordinator::willQuitLevelSession() {
    logging::info("[macro/session] Quitting level session");
    recorder().markSessionEnded();
    replay().endLevelSession();
}

}
