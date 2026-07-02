#include <contracts>
#include <print>

auto handle_contract_violation(std::contracts::contract_violation const &Violation) -> void {
    std::println("Contract violation: {}", Violation.comment());
}