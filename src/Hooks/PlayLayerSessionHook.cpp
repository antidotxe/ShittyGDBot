#include <GdBot/Features/Macro/MacroSessionCoordinator.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(GdBotPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        gdbot::MacroSessionCoordinator::get().didInitializeLevel(level);
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        gdbot::MacroSessionCoordinator::get().didResetLevel(this);
    }

    void loadFromCheckpoint(CheckpointObject* checkpoint) {
        gdbot::MacroSessionCoordinator::get().willLoadCheckpoint(checkpoint);
        PlayLayer::loadFromCheckpoint(checkpoint);
    }

    void storeCheckpoint(CheckpointObject* checkpoint) {
        PlayLayer::storeCheckpoint(checkpoint);
        gdbot::MacroSessionCoordinator::get().didStoreCheckpoint();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        gdbot::MacroSessionCoordinator::get().willDestroyPlayer(player);
        PlayLayer::destroyPlayer(player, object);
    }

    void levelComplete() {
        gdbot::MacroSessionCoordinator::get().willCompleteLevel(this);
        PlayLayer::levelComplete();
    }

    void onQuit() {
        gdbot::MacroSessionCoordinator::get().willQuitLevelSession();
        PlayLayer::onQuit();
    }
};
