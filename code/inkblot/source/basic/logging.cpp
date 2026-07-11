#include <inkblot/basic/logging.hpp>

#include <mutex>
#include <vector>
#include <unordered_map>

namespace ink::log
{
    inline std::mutex SinksMutex;
    inline std::vector<sink> Sinks;

    inline std::mutex ThreadNamesMutex;
    inline std::unordered_map<os::thread_id, std::string> ThreadNames;

    auto push_record_to_sinks(record &Record) -> void
    {
        {
            auto Lock = std::scoped_lock{ThreadNamesMutex};
            if (auto It = ThreadNames.find(os::current_thread_id()); It != ThreadNames.end()) {
                Record.ThreadName = It->second;
            }
        }

        auto Lock = std::scoped_lock{SinksMutex};
        
        for (auto &Sink : Sinks) {
            Sink.Output(Record);
        }
    }

    auto register_logging_thread_name(std::string Name, os::thread_id ThreadId) noexcept -> void
    {
        auto Lock = std::scoped_lock{ThreadNamesMutex};

        const auto [_, Inserted] = ThreadNames.try_emplace(ThreadId, std::move(Name));
        contract_assert(Inserted);
    }

    auto unregister_logging_thread_name(os::thread_id ThreadId) noexcept -> void
    {
        auto Lock = std::scoped_lock{ThreadNamesMutex};

        const auto Erased = ThreadNames.erase(ThreadId);
        contract_assert(Erased == 1);
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