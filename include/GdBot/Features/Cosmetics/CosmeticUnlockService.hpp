#pragma once

#include <string>
#include <vector>

namespace gdbot {

enum class CosmeticUnlockState {
    kIdle,
    kSucceeded,
    kWarning,
    kError,
};

struct CosmeticUnlockCategoryResult {
    std::string label;
    int unlockedCount = 0;
};

struct CosmeticUnlockReport {
    CosmeticUnlockState state = CosmeticUnlockState::kIdle;
    std::string statusText = "No cosmetic unlocks applied yet.";
    std::vector<CosmeticUnlockCategoryResult> results;
};

class CosmeticUnlockService {
public:
    static CosmeticUnlockService& get();

    CosmeticUnlockService(CosmeticUnlockService const&) = delete;
    CosmeticUnlockService& operator=(CosmeticUnlockService const&) = delete;

    [[nodiscard]] CosmeticUnlockReport const& unlockAllCosmetics();

    [[nodiscard]] CosmeticUnlockReport const& getLastReport() const;

private:
    CosmeticUnlockService() = default;

    CosmeticUnlockReport lastReport_;
};

}
