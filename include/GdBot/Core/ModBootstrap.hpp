#pragma once

namespace gdbot {

class ModBootstrap {
public:
    static ModBootstrap& get();

    ModBootstrap(ModBootstrap const&) = delete;
    ModBootstrap& operator=(ModBootstrap const&) = delete;

    void initialize();

private:
    ModBootstrap() = default;

    bool isInitialized_ = false;
};

}
