//
// Created by f0xeri on 30.11.2023.
//

#ifndef SLANGCREFACTORED_DRIVER_HPP
#define SLANGCREFACTORED_DRIVER_HPP

#include "common.hpp"
#include "CompilerOptions.hpp"
#include "check/Context.hpp"
#include "opt/Optimizer.hpp"

namespace Slangc {

    class Driver {
        CompilerOptions options;

    public:
        Driver(int argc, char **argv) : options(argc, argv) {};
        ~Driver() = default;

        void run();
        std::unique_ptr<Context> processUnit(std::filesystem::path &filepath, bool isMainModule);
    };
} // Slangc

#endif //SLANGCREFACTORED_DRIVER_HPP
