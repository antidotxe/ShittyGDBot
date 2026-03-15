#include <GdBot/Core/ModBootstrap.hpp>
#include <GdBot/Core/Logging.hpp>
#include <GdBot/Overlay/ImGuiOverlay.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

namespace {

constexpr auto kOverlayButtonMargin = 36.0F;
constexpr auto kOverlayMenuZOrder = 100;
constexpr char kOverlayButtonSpriteFrame[] = "GJ_infoIcon_001.png";

CCMenuItemSpriteExtra* createOverlayButton(CCObject* target, SEL_MenuHandler selector) {
    auto* buttonSprite = CCSprite::createWithSpriteFrameName(kOverlayButtonSpriteFrame);
    if (buttonSprite == nullptr) {
        gdbot::logging::warning("GdBot could not create menu button sprite");
        return nullptr;
    }

    auto* button = CCMenuItemSpriteExtra::create(buttonSprite, target, selector);
    if (button == nullptr) {
        gdbot::logging::warning("GdBot could not create overlay button");
        return nullptr;
    }

    return button;
}

bool attachOverlayButton(MenuLayer* layer, CCObject* target, SEL_MenuHandler selector) {
    if (layer == nullptr) {
        return false;
    }

    auto* button = createOverlayButton(target, selector);
    if (button == nullptr) {
        return false;
    }

    auto* menu = CCMenu::create();
    if (menu == nullptr) {
        gdbot::logging::warning("GdBot could not create overlay menu");
        return false;
    }

    auto const winSize = CCDirector::sharedDirector()->getWinSize();
    menu->setPosition(CCPointZero);
    button->setPosition({winSize.width - kOverlayButtonMargin, kOverlayButtonMargin});
    menu->addChild(button);
    layer->addChild(menu, kOverlayMenuZOrder);
    return true;
}

}

class $modify(GdBotMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        gdbot::ModBootstrap::get().initialize();
        if (attachOverlayButton(this, this, menu_selector(GdBotMenuLayer::onOverlayButton))) {
            gdbot::logging::debug("[menu] Attached overlay entry button to MenuLayer");
        }
        return true;
    }

    void onOverlayButton(CCObject*) {
        gdbot::logging::debug("[menu] Overlay entry button pressed");
        gdbot::ImGuiOverlay::get().show();
    }
};
