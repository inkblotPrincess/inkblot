#include <inkblot/basic/logging.hpp>
#include <inkblot/basic/multi_array.hpp>

#include <print>

struct point {
    float X;
    float Y;
    float Z;
};

auto format_to(auto Out, const point &Point)
{
    return std::format_to(Out, "({}, {}, {})", Point.X, Point.Y, Point.Z);
}

auto log_print(const ink::log::record &Record) -> void
{
    std::print(
        "[{:%Y-%m-%d %H:%M:%S}] [{}] >> {} - {}({}:{})\n", 
        std::chrono::floor<std::chrono::milliseconds>(Record.Timestamp), 
        Record.Level, 
        Record.Message, 
        Record.SourceFile, 
        Record.SourceLine, 
        Record.SourceColumn
    );
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) -> int {
    ink::log::push_sink({.Name = "stdout", .Output = &log_print});

    auto Points = ink::multi_array<point, 5zu>{{
        .X = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f},
        .Y = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f},
        .Z = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f}
    }};

    const auto MyPoint = point{.X = 1.0f, .Y = 2.0f, .Z = 3.0f};
    ink::log::info("MyPoint = {}", MyPoint);

    for (auto Idx = 0zu; Idx < Points.size(); ++Idx) {
        auto Point = Points[Idx];

        *Point.X += static_cast<float>(Idx) * 0.1f;
        *Point.Y += static_cast<float>(Idx) * 0.25f;
        *Point.Z += static_cast<float>(Idx) * 0.5f;
    }

    for (auto Idx = 0zu; Idx < Points.size(); ++Idx) {
        const auto Point = Points[Idx];
        ink::log::info("Point #{} = ({}, {}, {})", Idx + 1zu, *Point.X, *Point.Y, *Point.Z);
    }

    for (const auto &X : Points.X) {
        ink::log::info("X = {}", X);
    }

    ink::log::info("Points[2].X == {}", Points.X[2zu]);

    return 0;
}