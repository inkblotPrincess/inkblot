#pragma once

#include <format>
#include <meta>
#include <string>
#include <type_traits>
#include <utility>

namespace ink
{
    namespace detail 
    {
        struct to_string_cpo
        {
            template <typename type>
                requires std::is_enum_v<type>
            constexpr auto operator()(const type &Obj) -> std::string
            {
                template for (constexpr auto EnumValue : std::define_static_array(std::meta::enumerators_of(^^type))) {
                    if (Obj == [:EnumValue:]) {
                        return std::string{std::meta::identifier_of(EnumValue)};
                    }
                }

                std::unreachable();
            }
        };

        constinit inline auto ToString = to_string_cpo{};

        template <typename type> 
        struct formatter 
        {
            constexpr auto parse(std::format_parse_context &FormatContext) 
            {
                return std::ranges::begin(FormatContext);
            }

            constexpr auto format(const type &Object, std::format_context &FormatContext) const 
            {
                return std::format_to(FormatContext.out(), "{}", ToString(Object));
            }
        };

        template <typename type>
        concept formattable = requires(type Obj) {
            { ToString(Obj) } -> std::convertible_to<std::string>;
        };
    } // namespace detail
} // namespace ink

template <ink::detail::formattable type> 
struct std::formatter<type> : ink::detail::formatter<type> 
{
};