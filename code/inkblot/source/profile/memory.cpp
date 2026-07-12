#include <inkblot/os/memory.hpp>
#include <inkblot/profile/memory.hpp>

#include <memory>
#include <new>

namespace ink 
{
    struct allocation_header 
    {
        void       *Pointer;
        std::size_t Size;
    };

    auto track_allocation(std::size_t Size) noexcept -> void 
    {
        profile::MemoryMetrics.TotalAllocations.fetch_add(1zu, std::memory_order_relaxed);
        profile::MemoryMetrics.LiveAllocations.fetch_add(1zu, std::memory_order_relaxed);

        profile::MemoryMetrics.TotalAllocatedBytes.fetch_add(Size, std::memory_order_relaxed);
        profile::MemoryMetrics.LiveAllocatedBytes.fetch_add(Size, std::memory_order_relaxed);
    }

    auto track_deallocation(std::size_t Size) noexcept -> void 
    {
        profile::MemoryMetrics.LiveAllocations.fetch_sub(1zu, std::memory_order_relaxed);
        profile::MemoryMetrics.LiveAllocatedBytes.fetch_sub(Size, std::memory_order_relaxed);
    }
} // namespace ink

auto operator new(std::size_t Count) -> void* 
{
    const auto Size = Count + sizeof(ink::allocation_header);

    auto *const Memory = ink::os::memory_allocate(Size);
    if (Memory == nullptr) {
        throw std::bad_alloc{};
    }

    auto *const Header = static_cast<ink::allocation_header *>(Memory);
    Header->Pointer = Memory;
    Header->Size = Size;

    ink::track_allocation(Size);

    return Header + 1zu;
}

auto operator new[](std::size_t Count) -> void* 
{
    return ::operator new(Count);
}

auto operator new(std::size_t Count, std::align_val_t AlignmentValue) -> void* 
{
    const auto Alignment = std::max(static_cast<std::size_t>(AlignmentValue), alignof(ink::allocation_header));

    const auto Size = Count + Alignment - 1zu + sizeof(ink::allocation_header);
    auto *const Raw = ink::os::memory_allocate(Size);
    if (Raw == nullptr) {
        throw std::bad_alloc{};
    }

    auto Start = static_cast<void *>(static_cast<char *>(Raw) + sizeof(ink::allocation_header));
    auto Space = Size - sizeof(ink::allocation_header);

    auto *const AlignedPtr = std::align(Alignment, Count, Start, Space);
    if (AlignedPtr == nullptr) {
        ink::os::memory_free(Raw);
        throw std::bad_alloc{};
    }

    auto *const Header = static_cast<ink::allocation_header *>(AlignedPtr) - 1u;
    Header->Pointer = Raw;
    Header->Size = Size;

    ink::track_allocation(Size);

    return AlignedPtr;
}

auto operator new[](std::size_t Count, std::align_val_t AlignmentValue) -> void* 
{
    return ::operator new(Count, AlignmentValue);
}

auto operator delete(void* Pointer) noexcept -> void 
{
    if (Pointer == nullptr) {
        return;
    }

    auto *const Header = static_cast<ink::allocation_header *>(Pointer) - 1zu;
    ink::track_deallocation(Header->Size);
    ink::os::memory_free(Header->Pointer);
}

auto operator delete[](void* Pointer) noexcept -> void 
{
    ::operator delete(Pointer);
}

auto operator delete(void* Pointer, std::size_t) noexcept -> void 
{
    ::operator delete(Pointer);
}

auto operator delete[](void* Pointer, std::size_t) noexcept -> void 
{
    ::operator delete(Pointer);
}

auto operator delete(void* Pointer, std::align_val_t) noexcept -> void 
{
    if (Pointer == nullptr) {
        return;
    }

    auto *const Header = static_cast<ink::allocation_header *>(Pointer) - 1u;
    ink::track_deallocation(Header->Size);
    ink::os::memory_free(Header->Pointer);
}

auto operator delete[](void* Pointer, std::align_val_t AlignmentValue) noexcept -> void 
{
    ::operator delete(Pointer, AlignmentValue);
}

auto operator delete(void* Pointer, std::size_t, std::align_val_t AlignmentValue) noexcept -> void 
{
    ::operator delete(Pointer, AlignmentValue);
}

auto operator delete[](void* Pointer, std::size_t, std::align_val_t AlignmentValue) noexcept -> void 
{
    ::operator delete(Pointer, AlignmentValue);
}