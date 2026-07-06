#pragma once

#include <algorithm>
#include <concepts>
#include <format>
#include <meta>
#include <ranges>
#include <type_traits>
#include <utility>

namespace ink
{
    template <typename out_type, typename type>
    concept has_format_to_member = requires(out_type Out, const type& Object) {
        { Object.format_to(Out) } -> std::same_as<out_type>;
    };

    template <typename out_type, typename type>
    concept has_format_to_free = requires(out_type Out, const type& Object) {
        { format_to(Out, Object) } -> std::same_as<out_type>;
    };

    struct format_to_cpo
    {
        template <typename out_type, typename type>
        requires has_format_to_member<out_type, type>
        constexpr auto operator()(out_type Out, const type &Object) const -> out_type
        {
            return Object.format_to(Out);
        }

        template <typename out_type, typename type>
        requires (!has_format_to_member<out_type, type> && has_format_to_free<out_type, type>)
        constexpr auto operator()(out_type Out, const type &Object) const -> out_type
        {
            return format_to(Out, Object);
        }

        template <typename out_type, typename type>
        requires std::is_enum_v<type>
        constexpr auto operator()(out_type Out, const type &Object) const -> out_type
        {
            template for (constexpr auto EnumValue : std::define_static_array(std::meta::enumerators_of(^^type)))
            {
                if (Object == [:EnumValue:]) {
                    return std::ranges::copy(std::meta::identifier_of(EnumValue), Out).out;
                }
            }

            std::unreachable();
        }
    };

    constinit inline auto FormatTo = format_to_cpo{};

    template <typename type>
    concept formattable = 
        std::is_enum_v<type> || 
        requires(const type &Object, std::format_context &Context) {
            { FormatTo(Context.out(), Object) } -> std::same_as<std::format_context::iterator>;
        };

    template <typename type>
    struct formatter
    {
        constexpr auto parse(std::format_parse_context &Context)
        {
            return Context.begin();
        }

        auto format(const type &Object, std::format_context &Context) const
        {
            return FormatTo(Context.out(), Object);
        }
    };
} // namespace ink

template <ink::formattable type>
struct std::formatter<type> : ink::formatter<type>
{
};