#pragma once

#include <Geode/Enums.hpp>
#include <GdBot/Features/Macro/InputTypes.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

class GJGameLevel;
class CheckpointObject;
class PlayerObject;

namespace gdbot {

enum class InputLogEntryType {
    kMarker,
    kInput,
};

struct InputLogEntry {
    InputLogEntryType type = InputLogEntryType::kInput;
    std::string text;
    double timeSeconds = 0.0;
    PlayerButton button = PlayerButton::Jump;
    InputEdge edge = InputEdge::kPress;
    bool isPlayer2 = false;
};

struct InputRecorderSession {
    std::string levelName = "No active level";
    int attemptIndex = 1;
    bool isActive = false;
};

struct SavedRecordingInfo {
    std::string levelName = "No saved macro";
    bool hasRecording = false;
};

struct RecordedInputEvent {
    double timeSeconds = 0.0;
    int progressStep = 0;
    PlayerButton button = PlayerButton::Jump;
    InputEdge edge = InputEdge::kPress;
    bool isPlayer2 = false;
};

class InputRecorder {
public:
    static InputRecorder& get();

    InputRecorder(InputRecorder const&) = delete;
    InputRecorder& operator=(InputRecorder const&) = delete;

    void beginLevelSession(GJGameLevel* level);
    void startRecording();
    void stopRecording();
    void markAttemptReset();
    void trimRecordingToCheckpoint(CheckpointObject* checkpoint);
    void clearActiveRecording();
    void markCheckpointPlaced();
    void markPlayerDied(bool isPlayer2);
    void markLevelComplete();
    void markSessionEnded();
    void clear();

    void captureInputBeforeDispatch(PlayerObject* player, PlayerButton button, InputEdge edge);
    void recordPlayerInput(PlayerObject* player, PlayerButton button, InputEdge edge);

    [[nodiscard]] bool isRecording() const;
    [[nodiscard]] std::vector<InputLogEntry> const& getEntries() const;
    [[nodiscard]] std::size_t getEntryCount() const;
    [[nodiscard]] InputRecorderSession const& getSession() const;
    [[nodiscard]] SavedRecordingInfo const& getSavedRecordingInfo() const;
    [[nodiscard]] std::vector<RecordedInputEvent> const& getRecordedInputs() const;
    [[nodiscard]] std::size_t getRecordedInputCount() const;

private:
    struct RecorderSessionState {
        InputRecorderSession info;
        bool hasInputInAttempt = false;
        bool hasLoggedDeathInAttempt = false;
    };

    InputRecorder() = default;

    void beginAttempt();
    void appendEntry(InputLogEntry entry);
    void appendMarker(std::string text);
    void appendSessionMarker(std::string_view text);
    void appendTimedMarker(std::string text);
    [[nodiscard]] bool belongsToCurrentPlayLayer(PlayerObject* player) const;
    void trimEntries();

    [[nodiscard]] static std::string formatEntryText(
        double timeSeconds,
        bool isPlayer2,
        PlayerButton button,
        InputEdge edge,
        int attemptIndex
    );
    [[nodiscard]] static std::string formatMarkerText(
        double timeSeconds,
        int attemptIndex,
        std::string_view text
    );
    [[nodiscard]] static std::string resolveLevelName(GJGameLevel* level);
    [[nodiscard]] static char const* buttonToString(PlayerButton button);
    [[nodiscard]] static int resolveMacroProgressStep(class PlayLayer* playLayer);

    std::vector<InputLogEntry> entries_;
    std::vector<RecordedInputEvent> activeRecording_;
    std::vector<RecordedInputEvent> savedRecording_;
    RecorderSessionState session_;
    SavedRecordingInfo savedRecordingInfo_;
    std::string recordingLevelName_ = "No active level";
    double lastCheckpointMarkerTime_ = -1.0;
    bool isRecording_ = false;
};

}
