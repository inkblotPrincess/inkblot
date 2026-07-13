#pragma once

#include <concepts>
#include <cstdint>
#include <cmath>

namespace ink::math
{
    template <typename type>
    concept arithmetic = std::integral<type> || std::floating_point<type>;

    template <arithmetic value_type>
    struct vector2
    {
        value_type X = value_type{};
        value_type Y = value_type{};

        constexpr vector2() noexcept = default;

        constexpr explicit vector2(value_type XYValue) noexcept
            : vector2{XYValue, XYValue}
        {
        }

        constexpr explicit vector2(value_type XValue, value_type YValue) noexcept
            : X{XValue}
            , Y{YValue}
        {
        }

        constexpr auto operator+=(const vector2 &Other) noexcept -> vector2&
        {
            X += Other.X;
            Y += Other.Y;
            return *this;
        }

        constexpr auto operator-=(const vector2 &Other) noexcept -> vector2&
        {
            X -= Other.X;
            Y -= Other.Y;
            return *this;
        }

        constexpr auto operator*=(value_type Scalar) noexcept -> vector2&
        {
            X *= Scalar;
            Y *= Scalar;
            return *this;
        }

        constexpr auto operator/=(value_type Scalar) noexcept -> vector2&
            pre(Scalar != value_type{})
        {
            X /= Scalar;
            Y /= Scalar;
            return *this;
        }

        [[nodiscard]] friend constexpr auto operator+(vector2 Left, const vector2 &Right) noexcept -> vector2
        {
            return Left += Right;
        }

        [[nodiscard]] friend constexpr auto operator-(vector2 Left, const vector2 &Right) noexcept -> vector2
        {
            return Left -= Right;
        }

        [[nodiscard]] friend constexpr auto operator*(vector2 Vector, value_type Scalar) noexcept -> vector2
        {
            return Vector *= Scalar;
        }

        [[nodiscard]] friend constexpr auto operator/(vector2 Vector, value_type Scalar) noexcept -> vector2
        {
            return Vector /= Scalar;
        }

        [[nodiscard]] constexpr auto operator==(const vector2 &) const noexcept -> bool = default;
    };

    using vec2f = vector2<float>;
    using vec2d = vector2<double>;
    using vec2i = vector2<std::int32_t>;
    using vec2u = vector2<std::uint32_t>;

    template <arithmetic value_type>
    struct vector3
    {
        value_type X = value_type{};
        value_type Y = value_type{};
        value_type Z = value_type{};

        constexpr vector3() noexcept = default;

        constexpr explicit vector3(value_type XYZValue) noexcept
            : vector3{XYZValue, XYZValue, XYZValue}
        {
        }

        constexpr explicit vector3(value_type XValue, value_type YValue, value_type ZValue) noexcept
            : X{XValue}
            , Y{YValue}
            , Z{ZValue}
        {
        }

        static constexpr auto cross(const vector3 &Left, const vector3 &Right) noexcept -> vector3
            requires std::is_signed_v<value_type>
        {
            const auto I = (Left.Y * Right.Z) - (Left.Z * Right.Y);
            const auto J = (Left.X * Right.Z) - (Left.Z * Right.X);
            const auto K = (Left.X * Right.Y) - (Left.Y * Right.X);

            return vector3{I, -J, K};
        }

        static constexpr auto dot(const vector3 &Left, const vector3 &Right) noexcept -> value_type
        {
            return (Left.X * Right.X) + (Left.Y * Right.Y) + (Left.Z * Right.Z);
        }

        static constexpr auto normalise(const vector3 &Vector) noexcept -> vector3
            requires std::floating_point<value_type>
        {
            const auto Length = Vector.length();
            contract_assert(Length != value_type{});

            return vector3{Vector.X / Length, Vector.Y / Length, Vector.Z / Length};
        }

        constexpr auto length() const noexcept -> value_type
            requires std::floating_point<value_type>
        {
            return std::hypot(X, Y, Z);
        }

        constexpr auto operator+=(const vector3 &Other) noexcept -> vector3&
        {
            X += Other.X;
            Y += Other.Y;
            Z += Other.Z;
            return *this;
        }

        constexpr auto operator-=(const vector3 &Other) noexcept -> vector3&
        {
            X -= Other.X;
            Y -= Other.Y;
            Z -= Other.Z;
            return *this;
        }

        constexpr auto operator*=(value_type Scalar) noexcept -> vector3&
        {
            X *= Scalar;
            Y *= Scalar;
            Z *= Scalar;
            return *this;
        }

        constexpr auto operator/=(value_type Scalar) noexcept -> vector3&
            pre(Scalar != value_type{})
        {
            X /= Scalar;
            Y /= Scalar;
            Z /= Scalar;
            return *this;
        }

        [[nodiscard]] friend constexpr auto operator+(vector3 Left, const vector3 &Right) noexcept -> vector3
        {
            return Left += Right;
        }

        [[nodiscard]] friend constexpr auto operator-(vector3 Left, const vector3 &Right) noexcept -> vector3
        {
            return Left -= Right;
        }

        [[nodiscard]] friend constexpr auto operator*(vector3 Vector, value_type Scalar) noexcept -> vector3
        {
            return Vector *= Scalar;
        }

        [[nodiscard]] friend constexpr auto operator/(vector3 Vector, value_type Scalar) noexcept -> vector3
        {
            return Vector /= Scalar;
        }

        [[nodiscard]] constexpr auto operator==(const vector3 &) const noexcept -> bool = default;
    };

    using vec3f = vector3<float>;
    using vec3d = vector3<double>;
    using vec3i = vector3<std::int32_t>;
    using vec3u = vector3<std::uint32_t>;

    template <arithmetic value_type>
    struct vector4
    {
        value_type X = value_type{};
        value_type Y = value_type{};
        value_type Z = value_type{};
        value_type W = value_type{};

        constexpr vector4() noexcept = default;

        constexpr explicit vector4(value_type XYZWValue) noexcept
            : vector4{XYZWValue, XYZWValue, XYZWValue, XYZWValue}
        {
        }

        constexpr explicit vector4(value_type XValue, value_type YValue, value_type ZValue, value_type WValue) noexcept
            : X{XValue}
            , Y{YValue}
            , Z{ZValue}
            , W{WValue}
        {
        }

        constexpr auto operator+=(const vector4 &Other) noexcept -> vector4 &
        {
            X += Other.X;
            Y += Other.Y;
            Z += Other.Z;
            W += Other.W;
            return *this;
        }

        constexpr auto operator-=(const vector4 &Other) noexcept -> vector4&
        {
            X -= Other.X;
            Y -= Other.Y;
            Z -= Other.Z;
            W -= Other.W;
            return *this;
        }

        constexpr auto operator*=(value_type Scalar) noexcept -> vector4&
        {
            X *= Scalar;
            Y *= Scalar;
            Z *= Scalar;
            W *= Scalar;
            return *this;
        }

        constexpr auto operator/=(value_type Scalar) noexcept -> vector4&
            pre(Scalar != value_type{})
        {
            X /= Scalar;
            Y /= Scalar;
            Z /= Scalar;
            W /= Scalar;
            return *this;
        }

        [[nodiscard]] friend constexpr auto operator+(vector4 Left, const vector4 &Right) noexcept -> vector4
        {
            return Left += Right;
        }

        [[nodiscard]] friend constexpr auto operator-(vector4 Left, const vector4 &Right) noexcept -> vector4
        {
            return Left -= Right;
        }

        [[nodiscard]] friend constexpr auto operator*(vector4 Vector, value_type Scalar) noexcept -> vector4
        {
            return Vector *= Scalar;
        }

        [[nodiscard]] friend constexpr auto operator/(vector4 Vector, value_type Scalar) noexcept -> vector4
        {
            return Vector /= Scalar;
        }

        [[nodiscard]] constexpr auto operator==(const vector4 &) const noexcept -> bool = default;
    };

    using vec4f = vector4<float>;
    using vec4d = vector4<double>;
    using vec4i = vector4<std::int32_t>;
    using vec4u = vector4<std::uint32_t>;
} // namespace ink::math