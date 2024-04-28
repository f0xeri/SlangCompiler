//
// Created by f0xeri on 04.05.2023.
//

#ifndef SLANGCREFACTORED_COMMON_HPP
#define SLANGCREFACTORED_COMMON_HPP

#include <cstdint>
#include <string>
#include <algorithm>
#include <functional>
#include <utility>
#include <filesystem>
#include <llvm/Support/raw_ostream.h>

#ifdef _WIN32
#include <windows.h>

#undef min
#undef max
namespace Slangc {
    static std::filesystem::path getCurrentProcessDirectory() {
        wchar_t buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, sizeof(buffer));
        return std::filesystem::path(buffer).parent_path();
    }
}
#elif __linux__
#include <linux/limits.h>
#include <unistd.h>
#include <cstdio>
namespace Slangc {
    static std::filesystem::path getCurrentProcessDirectory() {
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        return std::filesystem::path(std::string(result, (count > 0) ? count : 0)).parent_path();
    }
}
#elif __APPLE__
#include <mach-o/dyld.h>
namespace Slangc {
    static std::filesystem::path getCurrentProcessDirectory() {
        char buffer[PATH_MAX];
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0) {
            return std::filesystem::path(buffer).parent_path();
        }
        return std::filesystem::path();
    }
}
#endif

namespace Slangc {

#define SLANGC_LOG(...) Slangc::log() << LogMessage{__VA_ARGS__}
#define ERRORS_FOUND Slangc::log().errorsPrinted

    enum class BuiltInType : uint8_t {
        Void = 0,
        Int,
        Float,
        Real,
        Char,
        Bool,
    };

    static const std::unordered_map<std::string_view, BuiltInType> builtInTypes = {
            {"int",         BuiltInType::Int},
            {"real",        BuiltInType::Real},
            {"float",       BuiltInType::Float},
            {"bool",        BuiltInType::Bool},
            {"char",        BuiltInType::Char},
            {"void",        BuiltInType::Void}
    };

    static constexpr std::string getBuiltInTypeName(BuiltInType type) {
        switch (type) {
            case BuiltInType::Int:      return "int";
            case BuiltInType::Real:     return "real";
            case BuiltInType::Float:    return "float";
            case BuiltInType::Bool:     return "bool";
            case BuiltInType::Char:     return "char";
            case BuiltInType::Void:     return "void";
        }
    }

    static bool isBuiltInNonVoid(std::string_view type) {
        return builtInTypes.contains(type) && builtInTypes.at(type) != BuiltInType::Void;
    }

    struct SourceLoc {
        SourceLoc(std::uint64_t line, std::uint64_t column) : line(line), column(column) {}

        std::uint64_t line;
        std::uint64_t column;

        [[nodiscard]] std::string toString() const {
            return "(" + std::to_string(line) + "," + std::to_string(column) + ")";
        }
    };

    enum class LogLevel {
        Info,
        Error,
        Warn,
        Debug
    };

    class LogMessage {
    public:
        std::string message;
        std::string sourceFile;
        SourceLoc location;
        bool internal = false;
        LogLevel logLevel = LogLevel::Info;
        LogMessage(std::string sourceFile, std::string message, SourceLoc location, LogLevel logLevel = LogLevel::Info,
                   bool internal = false)
                : message(std::move(message)), sourceFile(std::move(sourceFile)), location(location), logLevel(logLevel), internal(internal) {}

        void print(auto &stream) const {
            stream << sourceFile;
            stream << "(" << location.line << "," << location.column << "): ";
            if (logLevel == LogLevel::Warn) {
                stream << "warning";
            } else {
                stream << "error";
            }
            if (internal) {
                stream << " (internal compiler)";
            }

            stream << ": " << message << "\n";
        }
    };

    class Logger {
        llvm::raw_ostream &os;
        inline static bool hasErrors;
    public:
        ~Logger() = default;
        auto operator<<(const std::string &message) -> Logger & {
            os << message;
            return *this;
        }

        auto operator<<(const LogMessage &message) -> Logger & {
            if (message.logLevel == LogLevel::Error) hasErrors = true;
            message.print(os);
            return *this;
        }

        explicit Logger(llvm::raw_ostream &os) : os(os) {}

        static bool errorsPrinted() {
            return hasErrors;
        }
    };

    static Logger& log() {
        static Logger logger(llvm::outs());
        return logger;
    }

    /*void SLANGC_LOG(auto&&... args) {
        Slangc::log() << LogMessage{args...};
    }*/

    /*bool ERRORS_FOUND() {
        return Slangc::Logger::errorsPrinted();
    }*/

    [[nodiscard]] static std::string_view takeWhile(std::string_view str, std::function<bool(char)> pred) {
        auto endIt = std::find_if_not(str.begin(), str.end(), std::move(pred));
        return str.substr(0, std::distance(str.begin(), endIt));
    }
}

#endif //SLANGCREFACTORED_COMMON_HPP
