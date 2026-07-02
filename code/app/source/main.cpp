#include <inkblot/basic/logging.hpp>
#include <inkblot/basic/multi_array.hpp>

#include <print>

struct point {
    float X;
    float Y;
    float Z;
};

struct stdout_sink : ink::logging_sink
{
    auto push(const ink::logging_record &Record) -> void override
    {
        std::println("[{}] >> {} - {}({}:{})", Record.Level, Record.Message, Record.SourceLocation.File, Record.SourceLocation.Line, Record.SourceLocation.Column);
    }
};

auto main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) -> int {
    ink::push_logging_sink("stdout", std::make_unique<stdout_sink>());

    auto Points = ink::multi_array<point, 5zu>::from(
        point{.X = 1.0f, .Y = 1.0f, .Z = 1.0f},
        point{.X = 2.0f, .Y = 2.0f, .Z = 2.0f},
        point{.X = 3.0f, .Y = 3.0f, .Z = 3.0f},
        point{.X = 4.0f, .Y = 4.0f, .Z = 4.0f},
        point{.X = 5.0f, .Y = 5.0f, .Z = 5.0f}
    );

    for (auto Idx = 0zu; Idx < Points.size(); ++Idx) {
        auto Point = Points[Idx];

        *Point.X += static_cast<float>(Idx) * 0.1f;
        *Point.Y += static_cast<float>(Idx) * 0.25f;
        *Point.Z += static_cast<float>(Idx) * 0.5f;
    }

    for (auto Idx = 0zu; Idx < Points.size(); ++Idx) {
        const auto Point = Points[Idx];
        ink::log_info(" - Point #{} = ({}, {}, {})", Idx + 1zu, *Point.X, *Point.Y, *Point.Z);
    }

    for (const auto &X : Points.values<^^point::X>()) {
        ink::log_info("X = {}", X);
    }

    ink::log_info("Points[2].X == {}", Points.values<^^point::X>()[2zu]);

    return 0;
}