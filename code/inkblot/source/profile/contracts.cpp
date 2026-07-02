#include <inkblot/basic/logging.hpp>

#include <contracts>

auto handle_contract_violation(std::contracts::contract_violation const &Violation) -> void {
    const auto SourceLocation = Violation.location();
    const auto File = [](std::string_view Path) noexcept {
        if (auto const Position = Path.find_last_of("/\\"); Position != std::string_view::npos) {
            return Path.substr(Position + 1zu);
        }

        return Path;
    }(SourceLocation.file_name());

    ink::log_fatal("Contract violation @ {}({}:{}): {}", File, SourceLocation.line(), SourceLocation.column(), Violation.comment());
}