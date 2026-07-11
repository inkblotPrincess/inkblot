#pragma once

#include <inkblot/basic/error.hpp>

#include <format>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <process.h>

namespace ink::os::win32
{
    template <typename... arg_types>
    auto ensure_os(bool Predicate, std::format_string<arg_types...> Format, arg_types &&... Arguments) -> void
    {
        if (const auto Error = ::GetLastError(); !Predicate && Error != 0u)
        {
            throw system_exception{std::error_code{static_cast<int>(Error), std::system_category()}, Format, std::forward<arg_types>(Arguments)...};
        }
    }
} // namespace ink::os::win32