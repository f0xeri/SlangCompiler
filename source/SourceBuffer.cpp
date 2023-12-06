//
// Created by f0xeri on 04.05.2023.
//

#include <fcntl.h>
#include <sys/stat.h>
#include <system_error>
#include <fstream>
#include <filesystem>
#include <codecvt>
#include "SourceBuffer.hpp"

namespace Slangc {

    auto SourceBuffer::CreateFromFile(std::string_view path) -> llvm::Expected<SourceBuffer> {
        std::string filename = std::string(path);
        if (!filename.ends_with(".sl")) {
            return llvm::make_error<llvm::StringError>(filename + " : slang source file extension must be .sl", llvm::inconvertibleErrorCode());
        }
        constexpr auto read_size = std::size_t{4096};
        auto stream = std::basic_ifstream<char32_t>{path.data()};
        stream.exceptions(std::ios_base::badbit);

        if (!stream) {
            return llvm::make_error<llvm::StringError>(filename + " : file not found", llvm::inconvertibleErrorCode());
        }

        auto result = std::u32string{};
        auto buffer = std::u32string(read_size, '\0');
        while (stream.read(&buffer[0], read_size)) {
            result.append(buffer, 0, stream.gcount());
        }
        result.append(buffer, 0, stream.gcount());
        std::string str;
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        str = converter.to_bytes(result);
        return SourceBuffer{std::move(filename), std::move(str)};
    }

    SourceBuffer::SourceBuffer(std::string filename, std::string text) : filename(std::move(filename)), text(std::move(text)) {}

    SourceBuffer::SourceBuffer(SourceBuffer &&arg) noexcept : filename(std::move(arg.filename)), text(std::move(arg.text)) {}
} // Slangc