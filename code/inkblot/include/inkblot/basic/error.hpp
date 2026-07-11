#pragma once

#include <format>
#include <stacktrace>
#include <stdexcept>
#include <system_error>

namespace ink
{
    class exception : public std::runtime_error
    {
      public:
        template <typename... arg_types>
        constexpr explicit exception(std::format_string<arg_types...> Format, arg_types &&...Arguments)
            : std::runtime_error{std::format(Format, std::forward<arg_types>(Arguments)...)}
            , m_What{std::format("{}\n{}", std::runtime_error::what(), std::stacktrace::current(1))}
        {
        }

        [[nodiscard]] constexpr auto what() const noexcept -> const char* override
        {
            return m_What.c_str();
        }

      private:
        std::string m_What;
    };

    class system_exception : public exception
    {
      public:
        template <typename... arg_types>
        constexpr explicit system_exception(std::error_code ErrorCode, std::format_string<arg_types...> Format, arg_types &&...Arguments)
            : exception{Format, std::forward<arg_types>(Arguments)...}
            , m_ErrorCode{ErrorCode}
        {
        }

        [[nodiscard]] constexpr auto error_code() const noexcept -> std::error_code
        {
            return m_ErrorCode;
        }

      private:
        std::error_code m_ErrorCode;
    };

    template <typename... arg_types>
    constexpr auto ensure(bool Predicate, std::format_string<arg_types...> Format, arg_types &&...Arguments) -> void
    {
        if (!Predicate)
        {
            throw exception{Format, std::forward<arg_types>(Arguments)...};
        }
    }
} // namespace ink