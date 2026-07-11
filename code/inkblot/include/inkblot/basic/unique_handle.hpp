#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace ink
{
    template <typename handle_type>
    concept unique_handle_type = std::is_trivially_copyable_v<handle_type>;

    template <typename deleter_type, typename handle_type>
    concept handle_deleter = requires(const deleter_type &Deleter, const handle_type &Handle) {
        { Deleter(Handle) } noexcept -> std::same_as<void>;
    };

    template <unique_handle_type handle_type, handle_deleter<handle_type> deleter_type, handle_type InvalidHandle = handle_type{}>
    class unique_handle
    {
      public:
        constexpr unique_handle() noexcept = default;

        constexpr explicit unique_handle(handle_type Handle) noexcept
            : m_Handle{Handle}
        {
        }

        constexpr ~unique_handle() noexcept 
        {
            reset();
        }
        
        unique_handle(const unique_handle &) = delete;
        auto operator=(const unique_handle &) -> unique_handle& = delete;

        constexpr unique_handle(unique_handle &&From) noexcept
            : m_Handle{From.release()}
        {
        }

        constexpr auto operator=(unique_handle &&From) noexcept -> unique_handle
        {
            if (this != &From) {
                reset(From.release());
            }

            return *this;
        }

        template <typename converted_type>
        [[nodiscard]] constexpr auto as() const noexcept -> converted_type
        {
            return static_cast<converted_type>(m_Handle);
        }

        [[nodiscard]] constexpr auto get() const noexcept -> handle_type
        {
            return m_Handle;
        }

        constexpr auto release() noexcept -> handle_type
            post(get() == InvalidHandle)
        {
            return std::exchange(m_Handle, InvalidHandle);
        }

        constexpr auto reset(const handle_type NewHandle = InvalidHandle) noexcept -> void
            post(get() == NewHandle)
        {
            if (const auto OldHandle = std::exchange(m_Handle, NewHandle); OldHandle != InvalidHandle) {
                deleter_type{}(OldHandle);
            }
        }

        [[nodiscard]] constexpr auto operator==(const unique_handle &Other) const noexcept -> bool
        {
            return m_Handle == Other.m_Handle;
        }

        [[nodiscard]] constexpr auto operator*() const noexcept -> handle_type
        {
            return m_Handle;
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return m_Handle != InvalidHandle;
        }

      private:
        handle_type m_Handle = InvalidHandle;
    };
} // namespace ink