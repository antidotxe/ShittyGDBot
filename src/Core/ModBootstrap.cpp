#include <GdBot/Features/Macro/BotController.hpp>
#include <GdBot/Core/Logging.hpp>
#include <GdBot/Core/ModBootstrap.hpp>

namespace gdbot {

ModBootstrap& ModBootstrap::get() {
    static ModBootstrap instance;
    return instance;
}

void ModBootstrap::initialize() {
    if (isInitialized_) {
        return;
    }

    BotController::get().disarm();
    isInitialized_ = true;
    logging::info("[bootstrap] Initialized mod runtime state");
}

}
