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

namespace Slangc {
    struct SourceLoc {
        SourceLoc(std::uint64_t line, std::uint64_t column) : line(line), column(column) {}
        std::uint64_t line;
        std::uint64_t column;

        [[nodiscard]] std::string toString() const {
            return "(" + std::to_string(line) + "," + std::to_string(column) + ")";
        }
    };

    class ErrorMessage {
    public:
        std::string message;
        SourceLoc location;
        bool isWarning = false;
        bool internal = false;

        ErrorMessage(std::string message, SourceLoc location, bool isWarning = false, bool internal = false)
                : message(std::move(message)), location(location), isWarning(isWarning), internal(internal) {}

        void print(auto &stream, std::string_view file) const {
            stream << file;
            stream << "(" << location.line << "," << location.column << "): ";
            if (isWarning) {
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

    // https://www.foonathan.net/2022/05/recursive-variant-box/
    /*template <typename T>
    class Box
    {
        // Wrapper over unique_ptr.
        std::unique_ptr<T> _impl;

    public:
        // Automatic construction from a `T`, not a `T*`.
        Box(T &&obj) : _impl(new T(std::move(obj))) {}
        Box(const T &obj) : _impl(new T(obj)) {}

        // Copy constructor copies `T`.
        Box(const Box &other) : Box(*other._impl) {}
        Box &operator=(const Box &other)
        {
            *_impl = *other._impl;
            return *this;
        }

        // unique_ptr destroys `T` for us.
        ~Box() = default;

        // Access propagates constness.
        T &operator*() { return *_impl; }
        const T &operator*() const { return *_impl; }

        T *operator->() { return _impl.get(); }
        const T *operator->() const { return _impl.get(); }
    };*/

    [[nodiscard]] static std::string_view takeWhile(std::string_view str, std::function<bool(char)> pred) {
        auto endIt = std::find_if_not(str.begin(), str.end(), std::move(pred));
        return str.substr(0, std::distance(str.begin(), endIt));
    }

    static void printErrorMessages(std::vector<ErrorMessage> &errors, auto &stream, std::string_view file) {
        for (auto &error : errors) {
            error.print(stream, file);
        }
    }
}

#endif //SLANGCREFACTORED_COMMON_HPP
