#pragma once

#include <inkblot/basic/formatter.hpp>

#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <format>
#include <memory>
#include <source_location>
#include <string_view>

namespace ink 
{
    enum class logging_level
    {
        debug,
        info,
        warn,
        error,
        fatal
    };

    struct source_location 
    {
        std::string_view File;
        std::uint32_t Line;
        std::uint32_t Column;
        
        [[nodiscard]] consteval static auto current(std::source_location SourceLocation = std::source_location::current()) noexcept -> source_location
        {
            auto const File = [](std::string_view Path) consteval noexcept {
                if (auto const Position = Path.find_last_of("/\\"); Position != std::string_view::npos) {
                    return Path.substr(Position + 1zu);
                }

                return Path;
            }(SourceLocation.file_name());

            return {
                .File   = File,
                .Line   = SourceLocation.line(),
                .Column = SourceLocation.column()
            };
        }
    };

    struct logging_record 
    {
        std::string_view Message;
        logging_level Level;
        source_location SourceLocation;
    };

    struct logging_sink
    {
        virtual ~logging_sink() = default;

        virtual auto push(const logging_record &Record) -> void = 0;
    };

    namespace detail
    {
        template <typename ...arg_types>
        struct logging_config 
        {
            std::format_string<arg_types...> Format;
            source_location SourceLocation;

            template <std::size_t Size>
            consteval logging_config(const char (&InFormat)[Size], source_location InSourceLocation = source_location::current())
                : Format{InFormat}
                , SourceLocation{InSourceLocation}
            {
            }
        };

        template <typename ...arg_types>
        logging_config(std::format_string<arg_types...>) -> logging_config<arg_types...>;

        auto push_log_record_to_sinks(const logging_record &Record) -> void;

        template <typename... arg_types>
        auto log_message(logging_level Level, detail::logging_config<std::type_identity_t<arg_types>...> Config, arg_types &&...Arguments) -> void
        {
            auto CharBuffer = std::array<char, 512>{};
            const auto FormatResult = std::format_to_n(CharBuffer.data(), CharBuffer.size(), Config.Format, std::forward<arg_types>(Arguments)...);

            const auto Record = logging_record{
                .Message        = std::string_view{CharBuffer.data(), std::min(static_cast<std::size_t>(FormatResult.size), CharBuffer.size())},
                .Level          = Level,
                .SourceLocation = Config.SourceLocation
            };

            push_log_record_to_sinks(Record);
        }
    } // namespace detail

    auto push_logging_sink(std::string Name, std::unique_ptr<logging_sink> Sink) -> void;
    auto pop_logging_sink(std::string_view Name) -> void;

    template <typename... arg_types>
    auto log_debug(detail::logging_config<std::type_identity_t<arg_types>...> Config, arg_types &&...Arguments) -> void
    {
        detail::log_message(logging_level::debug, Config, std::forward<arg_types>(Arguments)...);
    }

    template <typename... arg_types>
    auto log_warn(detail::logging_config<std::type_identity_t<arg_types>...> Config, arg_types &&...Arguments) -> void
    {
        detail::log_message(logging_level::warn, Config, std::forward<arg_types>(Arguments)...);
    }

    template <typename... arg_types>
    auto log_info(detail::logging_config<std::type_identity_t<arg_types>...> Config, arg_types &&...Arguments) -> void
    {
        detail::log_message(logging_level::info, Config, std::forward<arg_types>(Arguments)...);
    }

    template <typename... arg_types>
    auto log_error(detail::logging_config<std::type_identity_t<arg_types>...> Config, arg_types &&...Arguments) -> void
    {
        detail::log_message(logging_level::error, Config, std::forward<arg_types>(Arguments)...);
    }

    template <typename... arg_types>
    auto log_fatal(detail::logging_config<std::type_identity_t<arg_types>...> Config, arg_types &&...Arguments) -> void
    {
        detail::log_message(logging_level::fatal, Config, std::forward<arg_types>(Arguments)...);
    }
} // namespace ink