#include <inkblot/math/vector.hpp>

#include <gtest/gtest.h>

#include <concepts>
#include <type_traits>

namespace ink::math
{
    static_assert(arithmetic<int>);
    static_assert(arithmetic<unsigned int>);
    static_assert(arithmetic<float>);
    static_assert(arithmetic<double>);

    static_assert(!arithmetic<bool*>);
    static_assert(!arithmetic<void*>);

    static_assert(std::same_as<vec2f, vector2<float>>);
    static_assert(std::same_as<vec2d, vector2<double>>);
    static_assert(std::same_as<vec2i, vector2<std::int32_t>>);
    static_assert(std::same_as<vec2u, vector2<std::uint32_t>>);

    static_assert(std::same_as<vec3f, vector3<float>>);
    static_assert(std::same_as<vec3d, vector3<double>>);
    static_assert(std::same_as<vec3i, vector3<std::int32_t>>);
    static_assert(std::same_as<vec3u, vector3<std::uint32_t>>);

    static_assert(std::same_as<vec4f, vector4<float>>);
    static_assert(std::same_as<vec4d, vector4<double>>);
    static_assert(std::same_as<vec4i, vector4<std::int32_t>>);
    static_assert(std::same_as<vec4u, vector4<std::uint32_t>>);

    TEST(math_vector2_tests, default_constructor_zero_initialises)
    {
        const auto Vector = vector2<int>{};

        EXPECT_EQ(Vector.X, 0);
        EXPECT_EQ(Vector.Y, 0);
    }

    TEST(math_vector2_tests, scalar_constructor_initialises)
    {
        const auto Vector = vector2<int>{4};

        EXPECT_EQ(Vector.X, 4);
        EXPECT_EQ(Vector.Y, 4);
    }

    TEST(math_vector2_tests, component_constructor_initialises)
    {
        const auto Vector = vector2<int>{2, 3};

        EXPECT_EQ(Vector.X, 2);
        EXPECT_EQ(Vector.Y, 3);
    }

    TEST(math_vector2_tests, addition_assignment)
    {
        auto Left = vector2<int>{2, 3};
        Left += vector2<int>{5, 7};

        EXPECT_EQ(Left.X, 7);
        EXPECT_EQ(Left.Y, 10);
    }

    TEST(math_vector2_tests, addition_assignment_returns_self)
    {
        auto Left = vector2<int>{2, 3};
        auto &Result = (Left += vector2<int>{5, 7});

        EXPECT_EQ(&Result, &Left);
    }

    TEST(math_vector2_tests, subtraction_assignment)
    {
        auto Left = vector2<int>{8, 9};
        Left -= vector2<int>{3, 4};

        EXPECT_EQ(Left.X, 5);
        EXPECT_EQ(Left.Y, 5);
    }

    TEST(math_vector2_tests, multiplication_assignment)
    {
        auto Vector = vector2<int>{2, 3};
        Vector *= 4;

        EXPECT_EQ(Vector.X, 8);
        EXPECT_EQ(Vector.Y, 12);
    }

    TEST(math_vector2_tests, division_assignment)
    {
        auto Vector = vector2<int>{8, 12};
        Vector /= 4;

        EXPECT_EQ(Vector.X, 2);
        EXPECT_EQ(Vector.Y, 3);
    }

    TEST(math_vector2_tests, addition)
    {
        const auto Left = vector2<int>{2, 3};
        const auto Right = vector2<int>{5, 7};
        const auto Result = Left + Right;

        EXPECT_EQ(Result, (vector2<int>{7, 10}));
    }

    TEST(math_vector2_tests, subtraction)
    {
        const auto Left = vector2<int>{8, 9};
        const auto Right = vector2<int>{3, 4};
        const auto Result = Left - Right;

        EXPECT_EQ(Result, (vector2<int>{5, 5}));
    }

    TEST(math_vector2_tests, scalar_multiplication)
    {
        const auto Vector = vector2<int>{2, 3};
        const auto Result = Vector * 4;

        EXPECT_EQ(Result, (vector2<int>{8, 12}));
    }

    TEST(math_vector2_tests, scalar_division)
    {
        const auto Vector = vector2<int>{8, 12};
        const auto Result = Vector / 4;

        EXPECT_EQ(Result, (vector2<int>{2, 3}));
    }

    TEST(math_vector3_tests, default_constructor_zero_initialises)
    {
        const auto Vector = vector3<int>{};

        EXPECT_EQ(Vector.X, 0);
        EXPECT_EQ(Vector.Y, 0);
        EXPECT_EQ(Vector.Z, 0);
    }

    TEST(math_vector3_tests, scalar_constructor_initialises)
    {
        const auto Vector = vector3<int>{4};

        EXPECT_EQ(Vector.X, 4);
        EXPECT_EQ(Vector.Y, 4);
        EXPECT_EQ(Vector.Z, 4);
    }

    TEST(math_vector3_tests, component_constructor_initialises)
    {
        const auto Vector = vector3<int>{2, 3, 4};

        EXPECT_EQ(Vector.X, 2);
        EXPECT_EQ(Vector.Y, 3);
        EXPECT_EQ(Vector.Z, 4);
    }

    TEST(math_vector3_tests, addition_assignment)
    {
        auto Left = vector3<int>{1, 2, 3};
        Left += vector3<int>{4, 5, 6};

        EXPECT_EQ(Left.X, 5);
        EXPECT_EQ(Left.Y, 7);
        EXPECT_EQ(Left.Z, 9);
    }

    TEST(math_vector3_tests, addition_assignment_returns_self)
    {
        auto Left = vector3<int>{1, 2, 3};
        auto &Result = (Left += vector3<int>{4, 5, 6});

        EXPECT_EQ(&Result, &Left);
    }

    TEST(math_vector3_tests, subtraction_assignment)
    {
        auto Left = vector3<int>{8, 9, 10};
        Left -= vector3<int>{3, 4, 5};

        EXPECT_EQ(Left.X, 5);
        EXPECT_EQ(Left.Y, 5);
        EXPECT_EQ(Left.Z, 5);
    }

    TEST(math_vector3_tests, multiplication_assignment)
    {
        auto Vector = vector3<int>{2, 3, 4};
        Vector *= 3;

        EXPECT_EQ(Vector.X, 6);
        EXPECT_EQ(Vector.Y, 9);
        EXPECT_EQ(Vector.Z, 12);
    }

    TEST(math_vector3_tests, division_assignment)
    {
        auto Vector = vector3<int>{6, 9, 12};
        Vector /= 3;

        EXPECT_EQ(Vector.X, 2);
        EXPECT_EQ(Vector.Y, 3);
        EXPECT_EQ(Vector.Z, 4);
    }

    TEST(math_vector3_tests, addition)
    {
        const auto Left = vector3<int>{1, 2, 3};
        const auto Right = vector3<int>{4, 5, 6};
        const auto Result = Left + Right;

        EXPECT_EQ(Result, (vector3<int>{5, 7, 9}));
    }

    TEST(math_vector3_tests, subtraction)
    {
        const auto Left = vector3<int>{8, 9, 10};
        const auto Right = vector3<int>{3, 4, 5};
        const auto Result = Left - Right;

        EXPECT_EQ(Result, (vector3<int>{5, 5, 5}));
    }

    TEST(math_vector3_tests, scalar_multiplication)
    {
        const auto Vector = vector3<int>{2, 3, 4};
        const auto Result = Vector * 3;

        EXPECT_EQ(Result, (vector3<int>{6, 9, 12}));
    }

    TEST(math_vector3_tests, scalar_division)
    {
        const auto Vector = vector3<int>{6, 9, 12};
        const auto Result = Vector / 3;

        EXPECT_EQ(Result, (vector3<int>{2, 3, 4}));
    }

    TEST(math_vector3_tests, dot)
    {
        const auto Left = vector3<int>{1, 2, 3};
        const auto Right = vector3<int>{4, 5, 6};
        const auto Result = vector3<int>::dot(Left, Right);

        EXPECT_EQ(Result, 32);
    }

    TEST(math_vector3_tests, dot_perpendicular_vectors)
    {
        const auto Left = vector3<int>{1, 0, 0};
        const auto Right = vector3<int>{0, 1, 0};
        const auto Result = vector3<int>::dot(Left, Right);

        EXPECT_EQ(Result, 0);
    }

    TEST(math_vector3_tests, dot_with_self)
    {
        const auto Vector = vector3<int>{2, 3, 6};
        const auto Result = vector3<int>::dot(Vector, Vector);

        EXPECT_EQ(Result, 49);
    }

    TEST(math_vector3_tests, cross)
    {
        const auto Left = vector3<int>{1, 0, 0};
        const auto Right = vector3<int>{0, 1, 0};
        const auto Result = vector3<int>::cross(Left, Right);

        EXPECT_EQ(Result, (vector3<int>{0, 0, 1}));
    }

    TEST(math_vector3_tests, cross_reversed)
    {
        const auto Left = vector3<int>{0, 1, 0};
        const auto Right = vector3<int>{1, 0, 0};
        const auto Result = vector3<int>::cross(Left, Right);

        EXPECT_EQ(Result, (vector3<int>{0, 0, -1}));
    }

    TEST(math_vector3_tests, cross_parallel_vectors)
    {
        const auto Left = vector3<int>{1, 2, 3};
        const auto Right = vector3<int>{2, 4, 6};
        const auto Result = vector3<int>::cross(Left, Right);

        EXPECT_EQ(Result, (vector3<int>{0, 0, 0}));
    }

    TEST(math_vector3_tests, cross_is_perpendicular_to_left)
    {
        const auto Left = vector3<int>{1, 2, 3};
        const auto Right = vector3<int>{4, 5, 6};
        const auto Cross = vector3<int>::cross(Left, Right);
        const auto Result = vector3<int>::dot(Cross, Left);

        EXPECT_EQ(Result, 0);
    }

    TEST(math_vector3_tests, cross_is_perpendicular_to_right)
    {
        const auto Left = vector3<int>{1, 2, 3};
        const auto Right = vector3<int>{4, 5, 6};
        const auto Cross = vector3<int>::cross(Left, Right);
        const auto Result = vector3<int>::dot(Cross, Right);

        EXPECT_EQ(Result, 0);
    }

    TEST(math_vector3_tests, length)
    {
        const auto Vector = vector3<float>{2.0F, 3.0F, 6.0F};
        const auto Result = Vector.length();

        EXPECT_FLOAT_EQ(Result, 7.0F);
    }

    TEST(math_vector3_tests, normalise)
    {
        const auto Vector = vector3<float>{0.0F, 3.0F, 4.0F};
        const auto Result = vector3<float>::normalise(Vector);

        EXPECT_FLOAT_EQ(Result.X, 0.0F);
        EXPECT_FLOAT_EQ(Result.Y, 0.6F);
        EXPECT_FLOAT_EQ(Result.Z, 0.8F);
    }

    TEST(math_vector3_tests, normalise_produces_unit_length)
    {
        const auto Vector = vector3<float>{2.0F, 3.0F, 6.0F};
        const auto Result = vector3<float>::normalise(Vector);

        EXPECT_NEAR(Result.length(), 1.0F, 0.00001F);
    }

    TEST(math_vector4_tests, default_constructor_zero_initialises)
    {
        const auto Vector = vector4<int>{};

        EXPECT_EQ(Vector.X, 0);
        EXPECT_EQ(Vector.Y, 0);
        EXPECT_EQ(Vector.Z, 0);
        EXPECT_EQ(Vector.W, 0);
    }

    TEST(math_vector4_tests, scalar_constructor_initialises)
    {
        const auto Vector = vector4<int>{4};

        EXPECT_EQ(Vector.X, 4);
        EXPECT_EQ(Vector.Y, 4);
        EXPECT_EQ(Vector.Z, 4);
        EXPECT_EQ(Vector.W, 4);
    }

    TEST(math_vector4_tests, component_constructor_initialises)
    {
        const auto Vector = vector4<int>{1, 2, 3, 4};

        EXPECT_EQ(Vector.X, 1);
        EXPECT_EQ(Vector.Y, 2);
        EXPECT_EQ(Vector.Z, 3);
        EXPECT_EQ(Vector.W, 4);
    }

    TEST(math_vector4_tests, addition_assignment)
    {
        auto Left = vector4<int>{1, 2, 3, 4};
        Left += vector4<int>{5, 6, 7, 8};

        EXPECT_EQ(Left.X, 6);
        EXPECT_EQ(Left.Y, 8);
        EXPECT_EQ(Left.Z, 10);
        EXPECT_EQ(Left.W, 12);
    }

    TEST(math_vector4_tests, addition_assignment_returns_self)
    {
        auto Left = vector4<int>{1, 2, 3, 4};
        auto &Result = (Left += vector4<int>{5, 6, 7, 8});

        EXPECT_EQ(&Result, &Left);
    }

    TEST(math_vector4_tests, subtraction_assignment)
    {
        auto Left = vector4<int>{9, 10, 11, 12};
        Left -= vector4<int>{4, 5, 6, 7};

        EXPECT_EQ(Left.X, 5);
        EXPECT_EQ(Left.Y, 5);
        EXPECT_EQ(Left.Z, 5);
        EXPECT_EQ(Left.W, 5);
    }

    TEST(math_vector4_tests, multiplication_assignment)
    {
        auto Vector = vector4<int>{1, 2, 3, 4};
        Vector *= 3;

        EXPECT_EQ(Vector.X, 3);
        EXPECT_EQ(Vector.Y, 6);
        EXPECT_EQ(Vector.Z, 9);
        EXPECT_EQ(Vector.W, 12);
    }

    TEST(math_vector4_tests, division_assignment)
    {
        auto Vector = vector4<int>{3, 6, 9, 12};
        Vector /= 3;

        EXPECT_EQ(Vector.X, 1);
        EXPECT_EQ(Vector.Y, 2);
        EXPECT_EQ(Vector.Z, 3);
        EXPECT_EQ(Vector.W, 4);
    }

    TEST(math_vector4_tests, addition)
    {
        const auto Left = vector4<int>{1, 2, 3, 4};
        const auto Right = vector4<int>{5, 6, 7, 8};
        const auto Result = Left + Right;

        EXPECT_EQ(Result, (vector4<int>{6, 8, 10, 12}));
    }

    TEST(math_vector4_tests, subtraction)
    {
        const auto Left = vector4<int>{9, 10, 11, 12};
        const auto Right = vector4<int>{4, 5, 6, 7};
        const auto Result = Left - Right;

        EXPECT_EQ(Result, (vector4<int>{5, 5, 5, 5}));
    }

    TEST(math_vector4_tests, scalar_multiplication)
    {
        const auto Vector = vector4<int>{1, 2, 3, 4};
        const auto Result = Vector * 3;

        EXPECT_EQ(Result, (vector4<int>{3, 6, 9, 12}));
    }

    TEST(math_vector4_tests, scalar_division)
    {
        const auto Vector = vector4<int>{3, 6, 9, 12};
        const auto Result = Vector / 3;

        EXPECT_EQ(Result, (vector4<int>{1, 2, 3, 4}));
    }
} // namespace ink::tests