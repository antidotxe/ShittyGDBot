#include <GdBot/Features/Macro/InputRecorder.hpp>
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include <utility>

using namespace geode::prelude;

namespace {

template <typename BaseCall>
bool forwardInputEdge(
    PlayerObject* player,
    PlayerButton button,
    gdbot::InputEdge edge,
    BaseCall&& baseCall
) {
    gdbot::InputRecorder::get().captureInputBeforeDispatch(player, button, edge);
    return std::forward<BaseCall>(baseCall)();
}

}

class $modify(GdBotPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton button) {
        return forwardInputEdge(this, button, gdbot::InputEdge::kPress, [&]() {
            return PlayerObject::pushButton(button);
        });
    }

    bool releaseButton(PlayerButton button) {
        return forwardInputEdge(this, button, gdbot::InputEdge::kRelease, [&]() {
            return PlayerObject::releaseButton(button);
        });
    }
};
