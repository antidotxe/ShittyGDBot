#include <GdBot/Overlay/ImGuiOverlay.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/CCEGLView.hpp>

using namespace geode::prelude;

class $modify(GdBotEglView, cocos2d::CCEGLView) {
    void swapBuffers() {
        gdbot::ImGuiOverlay::get().renderFrame();
        CCEGLView::swapBuffers();
    }
};
