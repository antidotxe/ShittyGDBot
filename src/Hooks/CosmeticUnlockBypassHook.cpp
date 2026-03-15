#include <GdBot/Core/Logging.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GameStatsManager.hpp>

using namespace geode::prelude;

namespace {

bool isGlowUnlockItem(UnlockType type, int itemId) {
    return type == UnlockType::GJItem && itemId >= 18 && itemId <= 20;
}

void logBypassUseOnce(char const* message, bool& hasLogged) {
    if (hasLogged) {
        return;
    }

    hasLogged = true;
    gdbot::logging::info("{}", message);
}

}

class $modify(GdBotUnlockIconsGameManager, GameManager) {
    bool isColorUnlocked(int id, UnlockType type) {
        auto const isUnlocked = GameManager::isColorUnlocked(id, type);
        if (isUnlocked || id <= 0) {
            return isUnlocked;
        }

        static auto hasLoggedColorBypass = false;
        logBypassUseOnce("[cosmetics] Runtime color unlock bypass is active", hasLoggedColorBypass);
        return true;
    }

    bool isIconUnlocked(int id, IconType type) {
        auto const isUnlocked = GameManager::isIconUnlocked(id, type);
        if (isUnlocked || id <= 0) {
            return isUnlocked;
        }

        static auto hasLoggedIconBypass = false;
        logBypassUseOnce("[cosmetics] Runtime icon unlock bypass is active", hasLoggedIconBypass);
        return true;
    }
};

class $modify(GdBotUnlockIconsStatsManager, GameStatsManager) {
    bool isItemUnlocked(UnlockType type, int id) {
        auto const isUnlocked = GameStatsManager::isItemUnlocked(type, id);
        if (isUnlocked || !isGlowUnlockItem(type, id)) {
            return isUnlocked;
        }

        static auto hasLoggedGlowBypass = false;
        logBypassUseOnce("[cosmetics] Runtime glow unlock bypass is active", hasLoggedGlowBypass);
        return true;
    }

    int getItemUnlockState(int itemID, UnlockType unlockType) {
        auto const unlockState = GameStatsManager::getItemUnlockState(itemID, unlockType);
        if (unlockState != 0) {
            return unlockState;
        }

        if (!isGlowUnlockItem(unlockType, itemID)) {
            return 0;
        }

        static auto hasLoggedGlowStateBypass = false;
        logBypassUseOnce("[cosmetics] Runtime glow state bypass is active", hasLoggedGlowStateBypass);
        return 1;
    }
};
