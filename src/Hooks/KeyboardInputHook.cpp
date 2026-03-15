#include <GdBot/Overlay/ImGuiOverlay.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;

class $modify(GdBotKeyboardDispatcher, cocos2d::CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double timestamp) {
        auto& overlay = gdbot::ImGuiOverlay::get();
        if (overlay.handleKeyboardInput(key, isKeyDown, isKeyRepeat)) {
            return true;
        }

        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, timestamp);
    }
};
