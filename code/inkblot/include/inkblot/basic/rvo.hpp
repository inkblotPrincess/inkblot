#pragma once

// This entire file exists as a result of a weird C++-ism where copy elision fails to occur for pair, optional,
// and expected types (among others). This solution uses a lambda-based trick to force the compiler to correctly 
// perform copy elision in these cases.
// 
// A demonstration of the issue can be found on Compiler Explorer: <https://godbolt.org/z/x6vjY5bce>.
//
// For more information on this issue and the solution:
//  - Jason Turner, 'C++ Weekly - Ep 421 - You're Using optional, variant, pair, tuple, any, and expected Wrong!': <https://www.youtube.com/watch?v=0yJk5yfdih0>
//  - Arthur O'Dwyer, 'The Superconstructing Super Elider': <https://quuxplusone.github.io/blog/2018/03/29/the-superconstructing-super-elider/>
//  - Arthur O'Dwyer, 'Superconstructing super elider, round 2': <https://quuxplusone.github.io/blog/2018/05/17/super-elider-round-2/>

// TODO: should we have macros for tuple, any, and variant too? - 7/July/26

#pragma once

#include <utility>

#define USE_WEIRD_RVO_TRICK 1

#if defined(USE_WEIRD_RVO_TRICK)
    #define MAKE_PAIR(F, S)     {::ink::with_result_of([&] { return F; }), ::ink::with_result_of([&] { return S; })}
    #define MAKE_OPTIONAL(O)    {::ink::with_result_of([&] { return O; })}
    #define MAKE_EXPECTED(E)    {::ink::with_result_of([&] { return E; })}
    #define MAKE_UNEXPECTED(U)  std::unexpected{::ink::with_result_of([&] { return U; })}
#else
    #define MAKE_PAIR(F, S)     A, B
    #define MAKE_OPTIONAL(O)    O
    #define MAKE_EXPECTED(E)    E
    #define MAKE_UNEXPECTED(U)  U
#endif

namespace ink
{
    template <typename func_type>
    class with_result_of_t 
    {
      public:
        using type = decltype(std::declval<func_type&&>()());

        explicit with_result_of_t(func_type&& Func) : m_Func{std::forward<func_type>(Func)} 
        {
        }

        operator type() 
        { 
            return m_Func(); 
        }

      private:
        func_type &&m_Func;
    };

    template <typename func_type>
    inline auto with_result_of(func_type &&Func) -> with_result_of_t<func_type> 
    {
        return with_result_of_t<func_type>(std::forward<func_type>(Func));
    }
} // namespace ink