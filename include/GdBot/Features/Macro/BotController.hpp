#pragma once

#include <string>

namespace gdbot {

class BotController {
public:
    static BotController& get();

    BotController(BotController const&) = delete;
    BotController& operator=(BotController const&) = delete;

    void arm();
    void disarm();
    void toggleArmed();

    [[nodiscard]] bool isArmed() const;
    [[nodiscard]] std::string getStatusText() const;

private:
    BotController() = default;

    bool isArmed_ = false;
};

}
