#include <GdBot/Overlay/ImGuiOverlay.hpp>
#include <GdBot/Core/Logging.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>

using namespace geode::prelude;

class $modify(GdBotDirector, cocos2d::CCDirector) {
    void purgeDirector() {
        gdbot::logging::debug("[overlay] CCDirector purge detected; shutting down overlay state");
        gdbot::ImGuiOverlay::get().shutdown();
        CCDirector::purgeDirector();
    }
};
