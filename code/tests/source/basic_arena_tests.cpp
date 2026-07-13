#include <inkblot/basic/arena.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>

namespace ink::tests
{
    namespace
    {
        struct test_object
        {
            std::int32_t X = 0;
            std::int32_t Y = 0;

            test_object() = default;

            explicit test_object(std::int32_t XValue, std::int32_t YValue)
                : X{XValue}
                , Y{YValue}
            {
            }
        };

        struct alignas(64) aligned_test_object
        {
            std::byte Value{};
        };

        struct non_trivially_destructible_object
        {
            ~non_trivially_destructible_object()
            {
            }
        };

        template <typename type>
        concept arena_allocatable = requires(arena &Arena) {
            Arena.allocate<type>();
        };
    } // namespace

    static_assert(arena_allocatable<int>);
    static_assert(arena_allocatable<test_object>);
    static_assert(!arena_allocatable<non_trivially_destructible_object>);

    static_assert(!std::default_initializable<arena>);
    static_assert(!std::copy_constructible<arena>);
    static_assert(!std::is_copy_assignable_v<arena>);
    static_assert(std::move_constructible<arena>);
    static_assert(std::is_move_assignable_v<arena>);

    TEST(basic_arena_tests, kilobytes_to_bytes_returns_byte_count)
    {
        EXPECT_EQ(kilobytes_to_bytes(1zu), 1024zu);
        EXPECT_EQ(kilobytes_to_bytes(4zu), 4096zu);
    }

    TEST(basic_arena_tests, megabytes_to_bytes_returns_byte_count)
    {
        EXPECT_EQ(megabytes_to_bytes(1zu), 1024zu * 1024zu);
        EXPECT_EQ(megabytes_to_bytes(4zu), 4zu * 1024zu * 1024zu);
    }

    TEST(basic_arena_tests, gigabytes_to_bytes_returns_byte_count)
    {
        EXPECT_EQ(gigabytes_to_bytes(1zu), 1024zu * 1024zu * 1024zu);
        EXPECT_EQ(gigabytes_to_bytes(4zu), 4zu * 1024zu * 1024zu * 1024zu);
    }

    TEST(basic_arena_tests, constructor_creates_single_block)
    {
        const auto Arena = arena{kilobytes_to_bytes(1zu)};

        EXPECT_EQ(Arena.size(), kilobytes_to_bytes(1zu));
        EXPECT_EQ(Arena.remaining(), kilobytes_to_bytes(1zu));
    }

    TEST(basic_arena_tests, allocate_bytesy)
    {
        auto Arena   = arena{kilobytes_to_bytes(1zu)};
        auto *Memory = Arena.allocate_bytes(128zu);

        EXPECT_NE(Memory, nullptr);
        EXPECT_EQ(Arena.remaining(), kilobytes_to_bytes(1zu) - 128zu);
    }

    TEST(basic_arena_tests, allocate_bytes_returns_non_overlapping_memory)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu)};

        auto *First  = static_cast<std::byte *>(Arena.allocate_bytes(128zu, 1zu));
        auto *Second = static_cast<std::byte *>(Arena.allocate_bytes(128zu, 1zu));

        ASSERT_NE(First, nullptr);
        ASSERT_NE(Second, nullptr);

        EXPECT_EQ(Second, First + 128zu);
    }

    TEST(basic_arena_tests, allocate_bytes_respects_requested_alignment)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu)};

        auto *Unaligned = static_cast<std::byte *>(Arena.allocate_bytes(1zu, 1zu));
        auto *Aligned   = static_cast<std::byte *>(Arena.allocate_bytes(64zu, 64zu));

        ASSERT_NE(Aligned, nullptr);

        const auto Address  = reinterpret_cast<std::uintptr_t>(Aligned);
        EXPECT_EQ(Address % 64zu, 0zu);

        const auto Consumed = static_cast<std::size_t>(Aligned - Unaligned) + 64zu;
        EXPECT_EQ(Arena.remaining(), kilobytes_to_bytes(1zu) - Consumed);
    }

    TEST(basic_arena_tests, do_not_grow)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu), arena::growth_policy::do_not_grow};

        [[maybe_unused]] auto *First = Arena.allocate_bytes(768zu, 1zu);
        const auto RemainingBeforeAllocation = Arena.remaining();

        auto *Second = Arena.allocate_bytes(512zu, 1zu);

        EXPECT_EQ(Second, nullptr);
        EXPECT_EQ(Arena.remaining(), RemainingBeforeAllocation);
        EXPECT_EQ(Arena.size(), kilobytes_to_bytes(1zu));
    }

    TEST(basic_arena_tests, chain_blocks)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu), arena::growth_policy::chain_blocks};

        [[maybe_unused]] auto *First = Arena.allocate_bytes(768zu, 1zu);
        auto *Second = Arena.allocate_bytes(512zu, 1zu);

        EXPECT_NE(Second, nullptr);
        EXPECT_EQ(Arena.remaining(), 512zu);
        EXPECT_EQ(Arena.size(), kilobytes_to_bytes(2zu));
    }

    TEST(basic_arena_tests, chain_blocks_can_create_multiple_blocks)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu),arena::growth_policy::chain_blocks};

        [[maybe_unused]] auto *First  = Arena.allocate_bytes(768zu, 1zu);
        [[maybe_unused]] auto *Second = Arena.allocate_bytes(768zu, 1zu);
        [[maybe_unused]] auto *Third  = Arena.allocate_bytes(768zu, 1zu);

        EXPECT_EQ(Arena.size(), kilobytes_to_bytes(3zu));
        EXPECT_EQ(Arena.remaining(), 256zu);
    }

    TEST(basic_arena_tests, allocate_constructs_object)
    {
        auto Arena   = arena{kilobytes_to_bytes(1zu)};
        auto *Object = Arena.allocate<test_object>(2, 3);

        ASSERT_NE(Object, nullptr);

        EXPECT_EQ(Object->X, 2);
        EXPECT_EQ(Object->Y, 3);
    }

    TEST(basic_arena_tests, allocate_default_constructs_object)
    {
        auto Arena   = arena{kilobytes_to_bytes(1zu)};
        auto *Object = Arena.allocate<test_object>();

        ASSERT_NE(Object, nullptr);

        EXPECT_EQ(Object->X, 0);
        EXPECT_EQ(Object->Y, 0);
    }

    TEST(basic_arena_tests, allocate_returns_null_when_memory_is_exhausted)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu), arena::growth_policy::do_not_grow};
        [[maybe_unused]] auto *Memory = Arena.allocate_bytes(1020zu, 1zu);

        auto *Object = Arena.allocate<test_object>();
        EXPECT_EQ(Object, nullptr);
    }

    TEST(basic_arena_tests, allocate_respects_type_alignment)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu)};

        [[maybe_unused]] auto *First = Arena.allocate<std::byte>();

        auto *Object = Arena.allocate<aligned_test_object>();
        ASSERT_NE(Object, nullptr);

        const auto Address = reinterpret_cast<std::uintptr_t>(Object);
        EXPECT_EQ(Address % alignof(aligned_test_object), 0zu);
    }

    TEST(basic_arena_tests, allocate_n_returns_requested_number_of_elements)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu)};

        const auto Elements = Arena.allocate_n<int>(8zu);
        EXPECT_EQ(Elements.size(), 8zu);
    }

    TEST(basic_arena_tests, allocate_n_returns_contiguous_elements)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu)};

        const auto Elements = Arena.allocate_n<int>(8zu);

        ASSERT_EQ(Elements.size(), 8zu);

        for (auto Index = 1zu; Index < Elements.size(); ++Index) {
            EXPECT_EQ(std::addressof(Elements[Index]), std::addressof(Elements[0]) + Index);
        }
    }

    TEST(basic_arena_tests, allocate_n_default_constructs_elements)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu)};

        const auto Elements = Arena.allocate_n<test_object>(8zu);
        ASSERT_EQ(Elements.size(), 8zu);

        for (const auto &Element : Elements) {
            EXPECT_EQ(Element.X, 0);
            EXPECT_EQ(Element.Y, 0);
        }
    }

    TEST(basic_arena_tests, allocate_n_returns_empty_span_when_memory_is_exhausted)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu), arena::growth_policy::do_not_grow};

        [[maybe_unused]] auto *Memory = Arena.allocate_bytes(1000zu, 1zu);
        const auto Elements = Arena.allocate_n<int>(8zu);

        EXPECT_TRUE(Elements.empty());
        EXPECT_EQ(Elements.data(), nullptr);
    }

    TEST(basic_arena_tests, allocated_elements_can_be_modified)
    {
        auto Arena = arena{kilobytes_to_bytes(1zu)};

        auto Elements = Arena.allocate_n<int>(4zu);
        ASSERT_EQ(Elements.size(), 4zu);

        Elements[0] = 2;
        Elements[1] = 3;
        Elements[2] = 5;
        Elements[3] = 7;

        EXPECT_EQ(Elements[0], 2);
        EXPECT_EQ(Elements[1], 3);
        EXPECT_EQ(Elements[2], 5);
        EXPECT_EQ(Elements[3], 7);
    }

    TEST(basic_arena_tests, move_constructor_transfers_blocks)
    {
        auto Source = arena{kilobytes_to_bytes(1zu), arena::growth_policy::chain_blocks};

        [[maybe_unused]] auto *First = Source.allocate_bytes(768zu, 1zu);
        [[maybe_unused]] auto *Second = Source.allocate_bytes(512zu, 1zu);

        const auto SizeBeforeMove = Source.size();
        const auto RemainingBeforeMove = Source.remaining();

        const auto Destination = arena{std::move(Source)};

        EXPECT_EQ(Destination.size(), SizeBeforeMove);
        EXPECT_EQ(Destination.remaining(), RemainingBeforeMove);
    }

    TEST(basic_arena_tests, move_assignment_transfers_blocks)
    {
        auto Source = arena{kilobytes_to_bytes(1zu), arena::growth_policy::chain_blocks};

        [[maybe_unused]] auto *First  = Source.allocate_bytes(768zu, 1zu);
        [[maybe_unused]] auto *Second = Source.allocate_bytes(512zu, 1zu);

        const auto SizeBeforeMove = Source.size();
        const auto RemainingBeforeMove = Source.remaining();

        auto Destination = arena{kilobytes_to_bytes(4zu)};
        Destination = std::move(Source);

        EXPECT_EQ(Destination.size(), SizeBeforeMove);
        EXPECT_EQ(Destination.remaining(), RemainingBeforeMove);
    }
} // namespace ink::tests