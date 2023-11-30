//
// Created by f0xeri on 30.11.2023.
//

#ifndef SLANGCREFACTORED_LOGGER_HPP
#define SLANGCREFACTORED_LOGGER_HPP

#include "llvm/Support/raw_ostream.h"
#include "common.hpp"

namespace Slangc {

    class Logger {
        llvm::raw_ostream &os;;
    public:
        ~Logger() = default;
        auto operator<<(const std::string &message) -> Logger & {
            os << message;
            return *this;
        }

        auto operator<<(const ErrorMessage &message) -> Logger & {
            message.print(os);
            return *this;
        }

        explicit Logger(llvm::raw_ostream &os) : os(os) {}
    };

    static Logger& log() {
        static Logger logger(llvm::outs());
        return logger;
    }
}



#endif //SLANGCREFACTORED_LOGGER_HPP
