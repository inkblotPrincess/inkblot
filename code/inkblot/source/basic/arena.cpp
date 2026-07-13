#include <inkblot/basic/arena.hpp>

#include <utility>

namespace ink
{
    arena::arena(std::size_t BlockSize, arena::growth_policy GrowthPolicy)
        : m_ArenaBlockHead{std::make_unique<arena_block>()}
        , m_GrowthPolicy{GrowthPolicy}
        , m_BlockSize{BlockSize}
    {
        m_ArenaBlockHead->Memory   = std::make_unique<std::byte[]>(m_BlockSize);
        m_ArenaBlockHead->Capacity = m_BlockSize;
    }

    auto arena::allocate_bytes(std::size_t Size, std::size_t Alignment) -> void*
    {
        auto *Block = m_ArenaBlockHead.get();
        while (Block != nullptr) {
            auto *Memory = Block->Memory.get() + Block->Offset;
            auto Space   = Block->Capacity - Block->Offset;

            auto *AlignedMemory = static_cast<void *>(Memory);
            if (std::align(Alignment, Size, AlignedMemory, Space) != nullptr) {
                auto *AlignedBytes = static_cast<std::byte *>(AlignedMemory);
                Block->Offset = static_cast<std::size_t>(AlignedBytes - Block->Memory.get()) + Size;
                
                return AlignedBytes;
            }

            if (Block->Next != nullptr) {
                Block = Block->Next.get();
                continue;
            }

            if (m_GrowthPolicy == arena::growth_policy::do_not_grow) {
                return nullptr;
            }

            Block->Next = std::make_unique<arena_block>();
            Block       = Block->Next.get();

            Block->Memory   = std::make_unique<std::byte[]>(m_BlockSize);
            Block->Capacity = m_BlockSize;
        }

        std::unreachable();
    }

    auto arena::remaining() const noexcept -> std::size_t
    {
        auto *Block = m_ArenaBlockHead.get();
        while (Block->Next != nullptr) {
            Block = Block->Next.get();
        }

        return Block->Capacity - Block->Offset;
    }

    auto arena::size() const noexcept -> std::size_t
    {
        // if we're never growing, we can shortcut the calculation
        if (m_GrowthPolicy == arena::growth_policy::do_not_grow) {
            return m_BlockSize;
        }

        auto Size = 0zu;
        
        auto *Block = m_ArenaBlockHead.get();
        while (Block != nullptr) {
            Size += Block->Capacity;
            Block = Block->Next.get();
        }

        return Size;
    }
} // namespace ink