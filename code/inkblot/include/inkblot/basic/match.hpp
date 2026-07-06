#pragma once

#include <variant>

namespace ink {
    template <typename ...types> 
    struct match : types... 
    {
        using types::operator()...;
    };

    template <typename ...types> 
    match(types...) -> match<types...>;

    template <class variant_type, typename matcher_type> 
    constexpr auto operator>>(variant_type &&Variant, matcher_type &&Matcher) -> decltype(auto) 
    {
        return std::visit(std::forward<matcher_type>(Matcher), std::forward<variant_type>(Variant));
    }
} // namespace ink
