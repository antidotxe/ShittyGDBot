#include <GdBot/Overlay/ImGuiOverlay.hpp>
#include <GdBot/Features/Macro/ReplayController.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(GdBotTouchDispatcher, cocos2d::CCTouchDispatcher) {
    void touchesBegan(cocos2d::CCSet* touches, cocos2d::CCEvent* event) {
        if (gdbot::ImGuiOverlay::get().isVisible()) {
            return;
        }

        cocos2d::CCTouchDispatcher::touchesBegan(touches, event);
    }

    void touchesMoved(cocos2d::CCSet* touches, cocos2d::CCEvent* event) {
        if (gdbot::ImGuiOverlay::get().isVisible()) {
            return;
        }

        cocos2d::CCTouchDispatcher::touchesMoved(touches, event);
    }

    void touchesEnded(cocos2d::CCSet* touches, cocos2d::CCEvent* event) {
        if (gdbot::ImGuiOverlay::get().isVisible()) {
            return;
        }

        cocos2d::CCTouchDispatcher::touchesEnded(touches, event);
    }

    void touchesCancelled(cocos2d::CCSet* touches, cocos2d::CCEvent* event) {
        if (gdbot::ImGuiOverlay::get().isVisible()) {
            return;
        }

        cocos2d::CCTouchDispatcher::touchesCancelled(touches, event);
    }
};

class $modify(GdBotMouseDispatcher, cocos2d::CCMouseDispatcher) {
    void dispatchScrollMSG(float x, float y) {
        if (gdbot::ImGuiOverlay::get().isVisible()) {
            return;
        }

        cocos2d::CCMouseDispatcher::dispatchScrollMSG(x, y);
    }
};

class $modify(GdBotBaseGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (gdbot::ImGuiOverlay::get().isVisible() &&
            !gdbot::ReplayController::get().isInjectingInput()) {
            return;
        }

        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};
