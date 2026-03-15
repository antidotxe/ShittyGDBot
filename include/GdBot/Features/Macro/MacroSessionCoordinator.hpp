#pragma once

class CheckpointObject;
class GJGameLevel;
class GJBaseGameLayer;
class PlayLayer;
class PlayerObject;

namespace gdbot {

class MacroSessionCoordinator {
public:
    static MacroSessionCoordinator& get();

    MacroSessionCoordinator(MacroSessionCoordinator const&) = delete;
    MacroSessionCoordinator& operator=(MacroSessionCoordinator const&) = delete;

    void didInitializeLevel(GJGameLevel* level);
    void didProcessQueuedButtons(GJBaseGameLayer* gameLayer);
    void didResetLevel(PlayLayer* playLayer);
    void willLoadCheckpoint(CheckpointObject* checkpoint);
    void didStoreCheckpoint();
    void willDestroyPlayer(PlayerObject* player);
    void willCompleteLevel(PlayLayer* playLayer);
    void willQuitLevelSession();

private:
    MacroSessionCoordinator() = default;
};

}
