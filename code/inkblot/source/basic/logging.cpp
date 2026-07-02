#include <inkblot/basic/logging.hpp>

#include <mutex>
#include <unordered_map>

namespace ink 
{
    inline std::mutex SinksMutex;
    inline std::unordered_map<std::string, std::unique_ptr<logging_sink>> Sinks;

    auto detail::push_log_record_to_sinks(const logging_record &Record) -> void
    {
        auto Lock = std::scoped_lock{SinksMutex};
        for (auto &[_, Sink] : Sinks) {
            Sink->push(Record);
        }
    }

    auto push_logging_sink(std::string Name, std::unique_ptr<logging_sink> Sink) -> void
    {
        auto Lock = std::scoped_lock{SinksMutex};
        
        const auto [_, Inserted] = Sinks.try_emplace(std::move(Name), std::move(Sink));
        contract_assert(Inserted);
    }
    
    auto pop_logging_sink(std::string_view Name) -> void
    {
        auto Lock = std::scoped_lock{SinksMutex};

        const auto It = std::ranges::find_if(Sinks, [&Name](const auto &Pair) noexcept {
            return Pair.first == Name;
        });
        contract_assert(It != Sinks.end());

        Sinks.erase(It);
    }
} // namespace ink