#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <meta>
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>

namespace ink 
{
    namespace detail 
    {
        template <typename type, std::meta::info Member>
        concept member_of = [] consteval {
            constexpr auto Cxt = std::meta::access_context::current();
            constexpr auto Members = std::define_static_array(std::meta::nonstatic_data_members_of(^^type, Cxt));

            return std::ranges::any_of(Members, [](const auto& TypeMember) consteval {
                return std::meta::identifier_of(TypeMember) == std::meta::identifier_of(Member);
            });
        }();

        template <typename type>
        concept multi_array_element = std::is_aggregate_v<type> && std::default_initializable<type>;

        template <typename element_type, std::size_t Size> 
        struct multi_array_storage
        {
            struct impl;

            consteval {
                const auto OldMembers = std::meta::nonstatic_data_members_of(^^element_type, std::meta::access_context::current());
                
                auto NewMembers = std::vector<std::meta::info>{};
                NewMembers.reserve(OldMembers.size());

                for (const auto &Member : OldMembers) {
                    const auto ArrayType = std::meta::substitute(^^std::array, {std::meta::type_of(Member), std::meta::reflect_constant(Size)});
                    NewMembers.emplace_back(std::meta::data_member_spec(ArrayType, {.name = std::meta::identifier_of(Member)}));
                }

                std::meta::define_aggregate(^^impl, NewMembers);
            }
        };

        template <typename self_type, typename element_type>
        struct multi_array_ref
        {
            struct impl;

            consteval {
                const auto OldMembers = std::meta::nonstatic_data_members_of(^^element_type, std::meta::access_context::current());
                
                auto NewMembers = std::vector<std::meta::info>{};
                NewMembers.reserve(OldMembers.size());

                for (const auto &Member : OldMembers) {
                    const auto MemberType = std::meta::type_of(Member);
                    const auto RefType = [&MemberType] consteval {
                        using std::meta::substitute;

                        if constexpr (std::is_const_v<std::remove_reference_t<self_type>>) {
                            return substitute(^^std::optional, {substitute(^^std::add_lvalue_reference_t, {substitute(^^std::add_const_t, {MemberType})})});
                        } else {
                            return substitute(^^std::optional, {substitute(^^std::add_lvalue_reference_t, {MemberType})});
                        }
                    }();

                    NewMembers.emplace_back(std::meta::data_member_spec(RefType, {.name = std::meta::identifier_of(Member)}));
                }

                std::meta::define_aggregate(^^impl, NewMembers);
            }
        };
    } // namespace detail

    template <detail::multi_array_element element_type, std::size_t Size>
    struct multi_array : private detail::multi_array_storage<element_type, Size>::impl 
    {
        using storage_type = detail::multi_array_storage<element_type, Size>::impl;

        template <typename ...element_types>
            requires (sizeof...(element_types) == Size && (std::same_as<std::remove_cvref_t<element_types>, element_type> && ...))
        [[nodiscard]] static constexpr auto from(element_types &&...Elements) noexcept -> multi_array
        {
            static constexpr auto Cxt = std::meta::access_context::current();
            static constexpr auto Members = std::define_static_array(std::meta::nonstatic_data_members_of(^^element_type, Cxt));
            static constexpr auto StorageMembers = std::define_static_array(std::meta::nonstatic_data_members_of(^^storage_type, Cxt));

            auto Result = multi_array{};
            
            auto ElementsTuple = std::forward_as_tuple(std::forward<element_types>(Elements)...);
            template for (constexpr auto Index : std::views::iota(0zu, Size)) {
                auto &&Element = std::get<Index>(ElementsTuple);

                template for (constexpr auto MemberIndex : std::views::iota(0zu, Members.size())) {
                    Result.[:StorageMembers[MemberIndex]:][Index] = std::forward<decltype(Element)>(Element).[:Members[MemberIndex]:];
                }
            }

            return Result;
        }

        [[nodiscard]] constexpr auto size() const noexcept -> std::size_t
        {
            return Size;
        }

        template <std::meta::info Member>
            requires detail::member_of<element_type, Member>
        [[nodiscard]] constexpr auto values(this auto& Self) noexcept
        {
            static constexpr auto StorageMember = [] consteval {
                static constexpr auto Cxt = std::meta::access_context::current();
                static constexpr auto StorageMembers = std::define_static_array(std::meta::nonstatic_data_members_of(^^storage_type, Cxt));

                // We know we can always dereference here since `Field` is pre-constrained by `detail::member_of<element_type, Field>`
                return *std::ranges::find_if(StorageMembers, [](const auto& TypeMember) consteval {
                    return std::meta::identifier_of(TypeMember) == std::meta::identifier_of(Member);
                });
            }();

            return std::span{Self.[:StorageMember:]};
        }

        [[nodiscard]] constexpr auto operator[](this auto &Self, std::size_t ElementIndex) noexcept
            pre(ElementIndex < Size)
        {
            using ref_type = detail::multi_array_ref<decltype(Self), element_type>::impl;

            static constexpr auto Cxt = std::meta::access_context::current();
            static constexpr auto Members = std::define_static_array(std::meta::nonstatic_data_members_of(^^ref_type, Cxt));
            static constexpr auto StorageMembers = std::define_static_array(std::meta::nonstatic_data_members_of(^^storage_type, Cxt));

            // Selina @ [2-July-26]
            // Currently, C++26's reflection does not support aggregate initialization of reference types generically via reflection.
            // 
            // We would like to do something like:
            //     return ref_type{
            //         template for (constexpr auto MemberIndex : std::views::iota(0zu, Members.size())) {
            //             .[:Members[MemberIndex]:] = Self.[:StorageMembers[MemberIndex]:][Index];
            //         }
            //     };
            // 
            // But since C++26 does not (a) support this syntax, (b) allow us to generically define a constructor to do this, or
            // (c) provide a utility function to mirror this functionality, we are forced to either use std::optional<T&> or a
            // system based on pointers. /shrug
            //
            // Similar functionality may have future support via P3294R2 ('Code Injection with Token Sequences') in C++29.
            // - <https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3294r2.html>
            // - <https://github.com/cplusplus/papers/issues/1946>
            auto Result = ref_type{};
            template for (constexpr auto MemberIndex : std::views::iota(0zu, Members.size())) {
                Result.[:Members[MemberIndex]:].emplace(Self.[:StorageMembers[MemberIndex]:][ElementIndex]);
            }

            return Result;
        }
    };
} // namespace ink