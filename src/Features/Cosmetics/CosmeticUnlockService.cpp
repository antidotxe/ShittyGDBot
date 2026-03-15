#include <GdBot/Features/Cosmetics/CosmeticUnlockService.hpp>
#include <GdBot/Core/Logging.hpp>

#include <Geode/Enums.hpp>
#include <Geode/binding/CharacterColorPage.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/binding/GameStatsManager.hpp>
#include <Geode/binding/GJStoreItem.hpp>

#include <array>
#include <fmt/format.h>
#include <set>
#include <string_view>
#include <vector>

namespace gdbot {

namespace {

struct CosmeticUnlockGroup {
    std::string_view label;
    IconType type;
};

constexpr auto kIconGroups = std::array{
    CosmeticUnlockGroup{"Cube", IconType::Cube},
    CosmeticUnlockGroup{"Ship", IconType::Ship},
    CosmeticUnlockGroup{"Ball", IconType::Ball},
    CosmeticUnlockGroup{"UFO", IconType::Ufo},
    CosmeticUnlockGroup{"Wave", IconType::Wave},
    CosmeticUnlockGroup{"Robot", IconType::Robot},
    CosmeticUnlockGroup{"Spider", IconType::Spider},
    CosmeticUnlockGroup{"Swing", IconType::Swing},
    CosmeticUnlockGroup{"Jetpack", IconType::Jetpack},
    CosmeticUnlockGroup{"Death effect", IconType::DeathEffect},
    CosmeticUnlockGroup{"Ship fire", IconType::ShipFire},
};

constexpr auto kGlowUnlockItemIds = std::array{18, 19, 20};

void collectColorIdsForCurrentMode(
    CharacterColorPage* colorPage,
    std::set<int>& seenColorIds,
    std::vector<int>& colorIds
) {
    if (colorPage == nullptr || colorPage->m_colorButtons == nullptr) {
        return;
    }

    auto const colorButtonCount = colorPage->m_colorButtons->count();
    colorIds.reserve(colorIds.size() + static_cast<size_t>(colorButtonCount));

    for (auto colorIndex = 0; colorIndex < colorButtonCount; ++colorIndex) {
        auto const colorId = colorPage->colorForIndex(colorIndex);
        if (colorId <= 0 || !seenColorIds.insert(colorId).second) {
            continue;
        }

        colorIds.push_back(colorId);
    }
}

std::vector<int> collectUnlockableColorIds() {
    std::vector<int> colorIds;

    auto* colorPage = CharacterColorPage::create();
    if (colorPage == nullptr) {
        logging::warning("GdBot could not create CharacterColorPage while enumerating player colors");
        return colorIds;
    }

    if (colorPage->m_colorButtons == nullptr) {
        logging::warning("GdBot found no color button dictionary while enumerating player colors");
        return colorIds;
    }

    std::set<int> seenColorIds;
    for (auto colorMode = 0; colorMode < 3; ++colorMode) {
        colorPage->updateColorMode(colorMode);
        collectColorIdsForCurrentMode(colorPage, seenColorIds, colorIds);
    }

    return colorIds;
}

void logReport(CosmeticUnlockReport const& report) {
    switch (report.state) {
        case CosmeticUnlockState::kSucceeded:
            logging::info("{}", report.statusText);
            break;

        case CosmeticUnlockState::kWarning:
            logging::warning("{}", report.statusText);
            break;

        case CosmeticUnlockState::kError:
            logging::error("{}", report.statusText);
            break;

        case CosmeticUnlockState::kIdle:
            break;
    }
}

int unlockIconGroups(
    GameManager* gameManager,
    std::vector<CosmeticUnlockCategoryResult>& results
) {
    auto unlockedCount = 0;

    for (auto const& group : kIconGroups) {
        auto const iconCount = gameManager->countForType(group.type);
        if (iconCount <= 0) {
            continue;
        }

        for (auto iconId = 1; iconId <= iconCount; ++iconId) {
            gameManager->unlockIcon(iconId, group.type);
            ++unlockedCount;
        }

        results.push_back({std::string(group.label), iconCount});
    }

    return unlockedCount;
}

int unlockColorGroup(
    GameManager* gameManager,
    std::string_view label,
    UnlockType type,
    std::vector<int> const& colorIds,
    std::vector<CosmeticUnlockCategoryResult>& results
) {
    if (colorIds.empty()) {
        return 0;
    }

    for (auto const colorId : colorIds) {
        gameManager->unlockColor(colorId, type);
    }

    results.push_back({std::string(label), static_cast<int>(colorIds.size())});
    return static_cast<int>(colorIds.size());
}

int unlockStoreCosmetics(
    GameStatsManager* gameStatsManager,
    std::vector<CosmeticUnlockCategoryResult>& results
) {
    if (gameStatsManager == nullptr) {
        logging::warning("GdBot could not access GameStatsManager while unlocking glow cosmetics");
        return 0;
    }

    if (gameStatsManager->m_allStoreItems == nullptr || gameStatsManager->m_unlockedItems == nullptr) {
        logging::warning("GdBot could not access store item dictionaries while unlocking glow cosmetics");
        return 0;
    }

    auto unlockedCount = 0;
    cocos2d::CCDictElement* element;
    cocos2d::CCDictElement* temp;
    HASH_ITER(hh, gameStatsManager->m_allStoreItems->m_pElements, element, temp) {
        auto* item = static_cast<GJStoreItem*>(element->getObject());
        if (item == nullptr) {
            continue;
        }

        auto const itemId = item->m_typeID.value();
        auto const unlockType = static_cast<UnlockType>(item->m_unlockType.value());
        if (unlockType != UnlockType::GJItem || itemId <= 0) {
            continue;
        }

        if (std::find(kGlowUnlockItemIds.begin(), kGlowUnlockItemIds.end(), itemId) != kGlowUnlockItemIds.end()) {
            continue;
        }

        auto const itemKey = gameStatsManager->getItemKey(itemId, static_cast<int>(unlockType));
        gameStatsManager->m_unlockedItems->setObject(cocos2d::CCString::create("1"), itemKey);
        ++unlockedCount;
    }

    if (unlockedCount > 0) {
        results.push_back({"Glow and store cosmetics", unlockedCount});
    }

    return unlockedCount;
}

int unlockGlowItems(
    GameStatsManager* gameStatsManager,
    std::vector<CosmeticUnlockCategoryResult>& results
) {
    if (gameStatsManager == nullptr) {
        return 0;
    }

    auto unlockedCount = 0;
    for (auto const itemId : kGlowUnlockItemIds) {
        gameStatsManager->toggleEnableItem(UnlockType::GJItem, itemId, true);
        ++unlockedCount;
    }

    results.push_back({"Glow unlock items", unlockedCount});
    return unlockedCount;
}

}

CosmeticUnlockService& CosmeticUnlockService::get() {
    static CosmeticUnlockService instance;
    return instance;
}

CosmeticUnlockReport const& CosmeticUnlockService::unlockAllCosmetics() {
    logging::info("[cosmetics] Starting cosmetic unlock pass");
    auto* gameManager = GameManager::get();
    if (gameManager == nullptr) {
        lastReport_ = {
            .state = CosmeticUnlockState::kError,
            .statusText = "Could not access GameManager to unlock cosmetics.",
        };
        logReport(lastReport_);
        return lastReport_;
    }

    auto report = CosmeticUnlockReport{};
    auto unlockedCount = unlockIconGroups(gameManager, report.results);

    auto const colorIds = collectUnlockableColorIds();
    logging::debug("[cosmetics] Enumerated {} unlockable player colors", colorIds.size());
    unlockedCount += unlockColorGroup(
        gameManager,
        "Primary color",
        UnlockType::Col1,
        colorIds,
        report.results
    );
    unlockedCount += unlockColorGroup(
        gameManager,
        "Secondary color",
        UnlockType::Col2,
        colorIds,
        report.results
    );

    auto* gameStatsManager = GameStatsManager::get();
    unlockedCount += unlockGlowItems(gameStatsManager, report.results);
    unlockedCount += unlockStoreCosmetics(gameStatsManager, report.results);

    if (report.results.empty()) {
        report.state = CosmeticUnlockState::kWarning;
        report.statusText = "GameManager reported no unlockable cosmetic categories.";
        lastReport_ = std::move(report);
        logReport(lastReport_);
        return lastReport_;
    }

    gameManager->checkUsedIcons();
    gameManager->save();
    logging::debug("[cosmetics] Saved cosmetic unlock state through GameManager");

    report.state = colorIds.empty() ? CosmeticUnlockState::kWarning : CosmeticUnlockState::kSucceeded;
    report.statusText = fmt::format(
        "Unlocked {} cosmetics across {} categories and saved the result.",
        unlockedCount,
        report.results.size()
    );
    if (colorIds.empty()) {
        report.statusText += " Player colors were unavailable to enumerate in this session.";
    }

    lastReport_ = std::move(report);
    logReport(lastReport_);
    return lastReport_;
}

CosmeticUnlockReport const& CosmeticUnlockService::getLastReport() const {
    return lastReport_;
}

}
