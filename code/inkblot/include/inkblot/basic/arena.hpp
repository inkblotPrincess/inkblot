#pragma once

#include <bit>
#include <cstddef>
#include <memory>
#include <span>

namespace ink
{
    [[nodiscard]] constexpr auto kilobytes_to_bytes(std::size_t Kilobytes) noexcept -> std::size_t
    {
        return Kilobytes * 1024;
    }
    
    [[nodiscard]] constexpr auto megabytes_to_bytes(std::size_t Megabytes) noexcept -> std::size_t
    {
        return Megabytes * 1024 * 1024;
    }
    
    [[nodiscard]] constexpr auto gigabytes_to_bytes(std::size_t Gigabytes) noexcept -> std::size_t
    {
        return Gigabytes * 1024 * 1024 * 1024;
    }

    class arena
    {
      public:
        enum class growth_policy
        {
            do_not_grow,
            chain_blocks
        };

        arena() = delete;
        explicit arena(std::size_t BlockSize, arena::growth_policy GrowthPolicy = arena::growth_policy::do_not_grow)
            pre(BlockSize >= kilobytes_to_bytes(1zu));

        ~arena() = default;

        arena(const arena &) = delete;
        auto operator=(const arena &) -> arena& = delete;

        arena(arena &&) noexcept = default;
        auto operator=(arena &&) noexcept -> arena& = default;

        template <typename type, typename... arg_types>
        requires std::constructible_from<type, arg_types...> && std::is_trivially_destructible_v<type>
        [[nodiscard]] auto allocate(arg_types &&...Arguments) -> type*
        {
            auto *Memory = allocate_bytes(sizeof(type), alignof(type));
            if (Memory == nullptr) {
                return nullptr;
            }

            return std::construct_at(static_cast<type *>(Memory), std::forward<arg_types>(Arguments)...);
        }

        template <typename type>
        requires std::is_nothrow_default_constructible_v<type> && std::is_trivially_destructible_v<type>
        [[nodiscard]] auto allocate_n(std::size_t Count) -> std::span<type>
            pre(Count > 0zu)
        {
            auto *Memory = allocate_bytes(sizeof(type) * Count, alignof(type));
            if (Memory == nullptr) {
                return {};
            }

            auto *Elements = static_cast<type *>(Memory);
            for (auto ElementsOffset = 0zu; ElementsOffset < Count; ++ElementsOffset) {
                std::construct_at(Elements + ElementsOffset);
            }

            return {Elements, Count};
        }

        [[nodiscard]] auto allocate_bytes(std::size_t Size, std::size_t Alignment = alignof(std::max_align_t)) -> void*
            pre(Size > 0zu)
            pre(Size < m_BlockSize)
            pre(std::has_single_bit(Alignment));


        [[nodiscard]] auto remaining() const noexcept -> std::size_t;

        [[nodiscard]] auto size() const noexcept -> std::size_t;

      private:
        struct arena_block
        {
            std::unique_ptr<arena_block> Next   = nullptr;
            std::unique_ptr<std::byte[]> Memory = nullptr;
            std::size_t Capacity = 0zu;
            std::size_t Offset   = 0zu;
        };

        std::unique_ptr<arena_block> m_ArenaBlockHead;
        arena::growth_policy         m_GrowthPolicy;
        std::size_t                  m_BlockSize;
    };
} // namespace ink