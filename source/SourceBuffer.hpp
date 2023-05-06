//
// Created by f0xeri on 04.05.2023.
//

#ifndef SLANGCREFACTORED_SOURCEBUFFER_HPP
#define SLANGCREFACTORED_SOURCEBUFFER_HPP

#include <utility>

#include "llvm/Support/Error.h"

namespace Slangc {

    class SourceBuffer {
    public:
        static auto CreateFromFile(std::string_view path) -> llvm::Expected<SourceBuffer>;
        SourceBuffer() = delete;
        SourceBuffer(const SourceBuffer& arg) = default;
        SourceBuffer(SourceBuffer&& arg) noexcept;
        ~SourceBuffer() = default;

        [[nodiscard]] auto getFilename() const -> std::string_view { return filename; }
        [[nodiscard]] auto getText() const -> std::string_view { return text; }

    private:
        std::string filename;
        std::string text;

        explicit SourceBuffer(std::string filename, std::string text);
    };

} // Slangc

#endif //SLANGCREFACTORED_SOURCEBUFFER_HPP
