#pragma once

#include <atomic>
#include <cstddef>

namespace ink::profile 
{
    struct memory_metrics 
    {
        std::atomic_size_t TotalAllocations;
        std::atomic_size_t LiveAllocations;

        std::atomic_size_t TotalAllocatedBytes;
        std::atomic_size_t LiveAllocatedBytes;
    };

    struct memory_metrics_snapshot 
    {
        std::size_t TotalAllocations;
        std::size_t LiveAllocations;

        std::size_t TotalAllocatedBytes;
        std::size_t LiveAllocatedBytes;
    };

    constinit inline auto MemoryMetrics = memory_metrics{};

    inline auto get_memory_metrics_snapshot() -> memory_metrics_snapshot 
    {
        return {
            .TotalAllocations    = MemoryMetrics.TotalAllocations.load(),
            .LiveAllocations     = MemoryMetrics.LiveAllocations.load(),
            .TotalAllocatedBytes = MemoryMetrics.TotalAllocatedBytes.load(),
            .LiveAllocatedBytes  = MemoryMetrics.LiveAllocatedBytes.load()
        };
    }
} // namespace ink::profile
