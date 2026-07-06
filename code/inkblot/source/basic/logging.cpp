#include <inkblot/basic/logging.hpp>

#include <mutex>
#include <vector>

namespace ink::log
{
    inline std::mutex SinksMutex;
    inline std::vector<sink> Sinks;

    auto push_record_to_sinks(const record &Record) -> void
    {
        auto Lock = std::scoped_lock{SinksMutex};
        
        for (auto &Sink : Sinks) {
            Sink.Output(Record);
        }
    }

    auto push_sink(sink Sink) -> void
    {
        auto Lock = std::scoped_lock{SinksMutex};

        contract_assert(std::ranges::none_of(Sinks, [Name = Sink.Name](const auto &SinkEntry) noexcept { return SinkEntry.Name == Name; }));
        Sinks.emplace_back(Sink);
    }
    
    auto pop_sink(std::string_view Name) -> void
    {
        auto Lock = std::scoped_lock{SinksMutex};

        const auto It = std::ranges::find_if(Sinks, [&Name](const auto &SinkEntry) noexcept { return SinkEntry.Name == Name; });
        contract_assert(It != Sinks.end());

        Sinks.erase(It);
    }
} // namespace ink::log