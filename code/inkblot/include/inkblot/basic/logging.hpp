#pragma once

#include <inkblot/basic/formatter.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <memory>
#include <source_location>
#include <string_view>

namespace ink::log
{
    enum class level
    {
        debug,
        info,
        warn,
        error,
        fatal
    };

    struct record 
    {
        std::string_view Message;
        level            Level;

        std::string_view SourceFile;
        std::uint32_t    SourceLine;
        std::uint32_t    SourceColumn;

        std::chrono::system_clock::time_point Timestamp;
    };

    struct sink
    {
        using sink_fn = void (*)(const record&);
        
        std::string_view Name;
        sink_fn          Output;
    };

    struct source_location 
    {
        std::string_view File;
        std::uint32_t    Line;
        std::uint32_t    Column;

        constexpr source_location(std::source_location SourceLocation)
            : File{[](std::string_view Path) noexcept {
                if (const auto Position = Path.find_last_of("/\\"); Position != std::string_view::npos) {
                    return Path.substr(Position + 1zu);
                }

                return Path;
            }(SourceLocation.file_name())}
            , Line{SourceLocation.line()}
            , Column{SourceLocation.column()}
        {
        }
        
        [[nodiscard]] consteval static auto current(std::source_location SourceLocation = std::source_location::current()) noexcept -> source_location
        {
            return source_location{SourceLocation};
        }
    };
    
    template <typename ...arg_types>
    struct logging_config 
    {
        std::format_string<arg_types...> Format;
        source_location          SourceLocation;

        template <std::size_t Size>
        consteval logging_config(const char (&InFormat)[Size], source_location InSourceLocation = source_location::current())
            : Format{InFormat}
            , SourceLocation{InSourceLocation}
        {
        }
    };

    template <typename ...arg_types>
    logging_config(std::format_string<arg_types...>) -> logging_config<arg_types...>;

    auto push_record_to_sinks(const record &Record) -> void;

    template <typename ...arg_types>
    auto log_message(level Level, std::format_string<arg_types...> Format, source_location SourceLocation, arg_types &&...Arguments) -> void
    {
        auto CharBuffer = std::array<char, 512>{};
        const auto FormatResult = std::format_to_n(CharBuffer.data(), CharBuffer.size(), Format, std::forward<arg_types>(Arguments)...);

        const auto Record = record{
            .Message      = std::string_view{CharBuffer.data(), std::min(static_cast<std::size_t>(FormatResult.size), CharBuffer.size())},
            .Level        = Level,
            .SourceFile   = SourceLocation.File,
            .SourceLine   = SourceLocation.Line,
            .SourceColumn = SourceLocation.Column,
            .Timestamp    = std::chrono::system_clock::now()
        };

        push_record_to_sinks(Record);
    }

    auto push_sink(sink Sink) -> void;
    auto pop_sink(std::string_view Name) -> void;

    template <typename... arg_types>
    inline auto debug(logging_config<std::type_identity_t<arg_types>...> logging_config, arg_types &&...Arguments) -> void
    {
        log_message(level::debug, logging_config.Format, logging_config.SourceLocation, std::forward<arg_types>(Arguments)...);
    }

    template <typename... arg_types>
    inline auto warn(logging_config<std::type_identity_t<arg_types>...> logging_config, arg_types &&...Arguments) -> void
    {
        log_message(level::warn, logging_config.Format, logging_config.SourceLocation, std::forward<arg_types>(Arguments)...);
    }

    template <typename... arg_types>
    inline auto info(logging_config<std::type_identity_t<arg_types>...> logging_config, arg_types &&...Arguments) -> void
    {
        log_message(level::info, logging_config.Format, logging_config.SourceLocation, std::forward<arg_types>(Arguments)...);
    }

    template <typename... arg_types>
    inline auto error(logging_config<std::type_identity_t<arg_types>...> logging_config, arg_types &&...Arguments) -> void
    {
        log_message(level::error, logging_config.Format, logging_config.SourceLocation, std::forward<arg_types>(Arguments)...);
    }

    template <typename... arg_types>
    inline auto fatal(logging_config<std::type_identity_t<arg_types>...> logging_config, arg_types &&...Arguments) -> void
    {
        log_message(level::fatal, logging_config.Format, logging_config.SourceLocation, std::forward<arg_types>(Arguments)...);
    }
} // namespace ink::log