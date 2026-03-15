#include <GdBot/Features/Macro/MacroSessionCoordinator.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(GdBotReplayDispatchLayer, GJBaseGameLayer) {
    void processQueuedButtons(float dt, bool clearInputQueue) {
        GJBaseGameLayer::processQueuedButtons(dt, clearInputQueue);
        gdbot::MacroSessionCoordinator::get().didProcessQueuedButtons(this);
    }
};
