#include <GdBot/Features/Macro/BotController.hpp>
#include <GdBot/Core/Logging.hpp>

namespace gdbot {

BotController& BotController::get() {
    static BotController instance;
    return instance;
}

void BotController::arm() {
    if (isArmed_) {
        return;
    }

    isArmed_ = true;
    logging::info("Playback armed");
}

void BotController::disarm() {
    if (!isArmed_) {
        return;
    }

    isArmed_ = false;
    logging::info("Playback disarmed");
}

void BotController::toggleArmed() {
    if (isArmed_) {
        disarm();
        return;
    }

    arm();
}

bool BotController::isArmed() const {
    return isArmed_;
}

std::string BotController::getStatusText() const {
    return isArmed_ ? "Playback status: running" : "Playback status: stopped";
}

}
