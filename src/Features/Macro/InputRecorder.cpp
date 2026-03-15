#include <GdBot/Features/Macro/InputRecorder.hpp>
#include <GdBot/Core/Logging.hpp>
#include <GdBot/Features/Macro/ReplayController.hpp>

#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/CheckpointObject.hpp>

#include <algorithm>
#include <fmt/format.h>
#include <sstream>

namespace gdbot {

namespace {

constexpr std::size_t kMaxEntries = 512;
constexpr double kCheckpointMarkerEpsilon = 0.001;

double resolveRecordingTime(PlayLayer* playLayer) {
    if (playLayer == nullptr) {
        return 0.0;
    }

    return playLayer->m_attemptTime;
}

std::string formatTime(double timeSeconds) {
    std::ostringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(3);
    stream << timeSeconds;
    return stream.str();
}

std::string formatRecordingSummary(
    std::string_view levelName,
    std::vector<RecordedInputEvent> const& events
) {
    if (events.empty()) {
        return fmt::format("No macro was saved for {}", levelName);
    }

    auto const& firstEvent = events.front();
    auto const& lastEvent = events.back();
    return fmt::format(
        "{} inputs saved for {} (duration {:.3f}s, progress {} -> {})",
        events.size(),
        levelName,
        lastEvent.timeSeconds,
        firstEvent.progressStep,
        lastEvent.progressStep
    );
}

}

InputRecorder& InputRecorder::get() {
    static InputRecorder instance;
    return instance;
}

void InputRecorder::beginLevelSession(GJGameLevel* level) {
    session_ = {};
    session_.info.levelName = resolveLevelName(level);
    session_.info.isActive = true;
    lastCheckpointMarkerTime_ = -1.0;
    logging::info("Recorder session started for {}", session_.info.levelName);
    if (isRecording_) {
        recordingLevelName_ = session_.info.levelName;
        entries_.clear();
        activeRecording_.clear();
        ReplayController::get().markRecordingDirty();
        beginAttempt();
    }
}

void InputRecorder::startRecording() {
    entries_.clear();
    activeRecording_.clear();
    savedRecording_.clear();
    savedRecordingInfo_ = {};
    recordingLevelName_ = session_.info.isActive ? session_.info.levelName : "No active level";
    isRecording_ = true;
    lastCheckpointMarkerTime_ = -1.0;
    session_.hasInputInAttempt = false;
    session_.hasLoggedDeathInAttempt = false;
    ReplayController::get().markRecordingDirty();
    logging::info("Recording started for {}", recordingLevelName_);

    if (session_.info.isActive) {
        beginAttempt();
    }
    else {
        appendMarker("Recording armed. Enter a level to capture inputs.");
    }
}

void InputRecorder::stopRecording() {
    if (!isRecording_) {
        return;
    }

    isRecording_ = false;
    savedRecording_ = activeRecording_;
    savedRecordingInfo_.hasRecording = !savedRecording_.empty();
    savedRecordingInfo_.levelName = savedRecordingInfo_.hasRecording ? recordingLevelName_ : "No saved macro";
    ReplayController::get().markRecordingDirty();
    logging::info("Recording stopped. {}", formatRecordingSummary(savedRecordingInfo_.levelName, savedRecording_));

    if (savedRecordingInfo_.hasRecording) {
        appendMarker(
            "Recording saved for " + savedRecordingInfo_.levelName +
            " (" + std::to_string(savedRecording_.size()) + " inputs)"
        );
    }
    else {
        appendMarker("Recording stopped with no inputs captured");
    }
}

void InputRecorder::markAttemptReset() {
    if (!isRecording_) {
        return;
    }

    if (!session_.hasInputInAttempt && !session_.hasLoggedDeathInAttempt) {
        return;
    }

    ++session_.info.attemptIndex;
    session_.hasInputInAttempt = false;
    session_.hasLoggedDeathInAttempt = false;
    logging::info(
        "Recorder moved to attempt {} for {}",
        session_.info.attemptIndex,
        session_.info.levelName
    );
    beginAttempt();
}

void InputRecorder::trimRecordingToCheckpoint(CheckpointObject* checkpoint) {
    if (!isRecording_ || checkpoint == nullptr) {
        return;
    }

    auto const checkpointProgressStep =
        static_cast<int>(checkpoint->m_gameState.m_currentProgress / kMacroProgressDivisor);
    auto const previousCount = activeRecording_.size();
    std::erase_if(activeRecording_, [checkpointProgressStep](RecordedInputEvent const& event) {
        return event.progressStep > checkpointProgressStep;
    });

    if (activeRecording_.size() == previousCount) {
        return;
    }

    logging::info(
        "Recording trimmed to checkpoint at step {} ({} -> {} inputs)",
        checkpointProgressStep,
        previousCount,
        activeRecording_.size()
    );
    appendMarker(
        fmt::format(
            "Recording trimmed to checkpoint at step {} ({} inputs remain)",
            checkpointProgressStep,
            activeRecording_.size()
        )
    );
}

void InputRecorder::clearActiveRecording() {
    if (!isRecording_ || activeRecording_.empty()) {
        return;
    }

    logging::info("Recording cleared after full reset ({} inputs removed)", activeRecording_.size());
    activeRecording_.clear();
    ReplayController::get().markRecordingDirty();
    appendMarker("Recording cleared after full reset");
}

void InputRecorder::markCheckpointPlaced() {
    if (!session_.info.isActive) {
        return;
    }

    auto const checkpointTime = resolveRecordingTime(PlayLayer::get());
    if (lastCheckpointMarkerTime_ >= 0.0 &&
        std::abs(checkpointTime - lastCheckpointMarkerTime_) <= kCheckpointMarkerEpsilon) {
        return;
    }

    lastCheckpointMarkerTime_ = checkpointTime;
    logging::info("Checkpoint placed at {:.3f}s", checkpointTime);
    appendSessionMarker("Checkpoint placed");
}

void InputRecorder::markPlayerDied(bool isPlayer2) {
    if (!isRecording_) {
        return;
    }

    if (session_.hasLoggedDeathInAttempt) {
        return;
    }

    session_.hasLoggedDeathInAttempt = true;
    logging::info(
        "{} death recorded on attempt {} at {:.3f}s",
        isPlayer2 ? "P2" : "P1",
        session_.info.attemptIndex,
        resolveRecordingTime(PlayLayer::get())
    );
    appendSessionMarker(isPlayer2 ? "P2 died" : "P1 died");
}

void InputRecorder::markLevelComplete() {
    if (!isRecording_) {
        return;
    }

    logging::info(
        "Level complete recorded for {} on attempt {}",
        session_.info.levelName,
        session_.info.attemptIndex
    );
    appendSessionMarker("Level complete");
}

void InputRecorder::markSessionEnded() {
    if (!isRecording_) {
        logging::info("Recorder session ended for {}", session_.info.levelName);
        session_ = {};
        return;
    }

    if (entries_.empty()) {
        session_ = {};
        activeRecording_.clear();
        ReplayController::get().markRecordingDirty();
        return;
    }

    appendMarker("Session ended");
    logging::info(
        "Recorder session ended for {} with {} live inputs captured",
        session_.info.levelName,
        activeRecording_.size()
    );
    session_ = {};
    ReplayController::get().markRecordingDirty();
}

void InputRecorder::clear() {
    entries_.clear();
    activeRecording_.clear();
    savedRecording_.clear();
    savedRecordingInfo_ = {};
    recordingLevelName_ = "No active level";
    lastCheckpointMarkerTime_ = -1.0;
    isRecording_ = false;
    ReplayController::get().markRecordingDirty();
    logging::info("Recorder terminal state cleared");

    if (session_.info.isActive) {
        session_.hasInputInAttempt = false;
        session_.hasLoggedDeathInAttempt = false;
        return;
    }

    session_ = {};
}

void InputRecorder::captureInputBeforeDispatch(PlayerObject* player, PlayerButton button, InputEdge edge) {
    if (ReplayController::get().isInjectingInput()) {
        return;
    }

    recordPlayerInput(player, button, edge);
}

void InputRecorder::recordPlayerInput(PlayerObject* player, PlayerButton button, InputEdge edge) {
    if (!isRecording_) {
        return;
    }

    if (!belongsToCurrentPlayLayer(player)) {
        return;
    }

    auto* playLayer = PlayLayer::get();
    session_.hasInputInAttempt = true;
    auto const recordingTime = resolveRecordingTime(playLayer);
    auto const progressStep = resolveMacroProgressStep(playLayer);

    InputLogEntry entry;
    entry.type = InputLogEntryType::kInput;
    entry.timeSeconds = recordingTime;
    entry.button = button;
    entry.edge = edge;
    entry.isPlayer2 = player->m_isSecondPlayer;
    entry.text = formatEntryText(
        entry.timeSeconds,
        entry.isPlayer2,
        entry.button,
        entry.edge,
        session_.info.attemptIndex
    );

    appendEntry(std::move(entry));
    activeRecording_.push_back({
        recordingTime,
        progressStep,
        button,
        edge,
        player->m_isSecondPlayer,
    });
    ReplayController::get().markRecordingDirty();
}

bool InputRecorder::isRecording() const {
    return isRecording_;
}

std::vector<InputLogEntry> const& InputRecorder::getEntries() const {
    return entries_;
}

std::size_t InputRecorder::getEntryCount() const {
    return entries_.size();
}

InputRecorderSession const& InputRecorder::getSession() const {
    return session_.info;
}

SavedRecordingInfo const& InputRecorder::getSavedRecordingInfo() const {
    return savedRecordingInfo_;
}

std::vector<RecordedInputEvent> const& InputRecorder::getRecordedInputs() const {
    return savedRecording_;
}

std::size_t InputRecorder::getRecordedInputCount() const {
    return savedRecording_.size();
}

void InputRecorder::beginAttempt() {
    if (!isRecording_) {
        return;
    }

    appendMarker("Attempt " + std::to_string(session_.info.attemptIndex) + " started");
}

void InputRecorder::appendEntry(InputLogEntry entry) {
    entries_.push_back(std::move(entry));
    trimEntries();
}

void InputRecorder::appendMarker(std::string text) {
    InputLogEntry entry;
    entry.type = InputLogEntryType::kMarker;
    entry.text = "> " + std::move(text);
    appendEntry(std::move(entry));
}

void InputRecorder::appendSessionMarker(std::string_view text) {
    appendTimedMarker(std::string(text));
}

void InputRecorder::appendTimedMarker(std::string text) {
    auto* playLayer = PlayLayer::get();
    if (playLayer == nullptr) {
        appendMarker(std::move(text));
        return;
    }

    InputLogEntry entry;
    entry.type = InputLogEntryType::kMarker;
    entry.timeSeconds = resolveRecordingTime(playLayer);
    entry.text = formatMarkerText(entry.timeSeconds, session_.info.attemptIndex, text);
    appendEntry(std::move(entry));
}

bool InputRecorder::belongsToCurrentPlayLayer(PlayerObject* player) const {
    auto* playLayer = PlayLayer::get();
    return player != nullptr && playLayer != nullptr && player->m_gameLayer == playLayer;
}

void InputRecorder::trimEntries() {
    if (entries_.size() > kMaxEntries) {
        entries_.erase(entries_.begin());
    }
}

std::string InputRecorder::formatEntryText(
    double timeSeconds,
    bool isPlayer2,
    PlayerButton button,
    InputEdge edge,
    int attemptIndex
) {
    std::ostringstream stream;
    stream
        << "[" << formatTime(timeSeconds) << "s]"
        << " [A" << attemptIndex << "] "
        << (isPlayer2 ? "P2" : "P1")
        << " "
        << toString(edge)
        << " "
        << buttonToString(button);
    return stream.str();
}

std::string InputRecorder::formatMarkerText(double timeSeconds, int attemptIndex, std::string_view text) {
    std::ostringstream stream;
    stream
        << "> [" << formatTime(timeSeconds) << "s]"
        << " [A" << attemptIndex << "] "
        << text;
    return stream.str();
}

std::string InputRecorder::resolveLevelName(GJGameLevel* level) {
    if (level == nullptr) {
        return "No active level";
    }

    auto levelName = std::string(level->m_levelName);
    if (levelName.empty()) {
        return "Unnamed level";
    }

    return levelName;
}

char const* InputRecorder::buttonToString(PlayerButton button) {
    switch (button) {
        case PlayerButton::Jump:
            return "Jump";
        case PlayerButton::Left:
            return "Left";
        case PlayerButton::Right:
            return "Right";
        default:
            return "Unknown";
    }
}

int InputRecorder::resolveMacroProgressStep(PlayLayer* playLayer) {
    if (playLayer == nullptr) {
        return 0;
    }

    return static_cast<int>(playLayer->m_gameState.m_currentProgress / kMacroProgressDivisor);
}

}
