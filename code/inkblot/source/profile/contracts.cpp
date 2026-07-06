#include <inkblot/basic/logging.hpp>

#include <contracts>

auto handle_contract_violation(std::contracts::contract_violation const &Violation) -> void {
    ink::log::log_message(ink::log::level::fatal, "Contract violation: {}", Violation.location(), Violation.comment());
}