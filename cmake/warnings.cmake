add_library(inkblot_warnings INTERFACE)

option(INKBLOT_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

set(MSVC_WARNINGS
    /W4
    /permissive-
    /w14242 # conversion, possible data loss
    /w14254 # operator conversion, possible data loss
    /w14263 # member function does not override base virtual function
    /w14265 # class has virtual functions but non-virtual destructor
    /w14287 # unsigned/negative constant mismatch
    /we4289 # nonstandard extension: loop control variable used outside loop scope
    /w14296 # expression is always true/false
    /w14311 # pointer truncation
    /w14545 # expression before comma has no effect
    /w14546 # function call before comma missing argument list
    /w14547 # operator before comma has no effect
    /w14549 # operator before comma has no effect (different variant)
    /w14555 # expression has no effect
    /w14619 # pragma warning: no warning number specified
    /w14640 # thread-unsafe static initialization
    /w14826 # sign extension conversion
    /w14905 # wide string literal cast to LPSTR
    /w14906 # string literal cast to LPWSTR
    /w14928 # illegal copy initialization / narrowing
)

set(CLANG_GCC_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion           # Implicit narrowing conversions
    -Wsign-conversion      # Signed/unsigned conversions
    -Wshadow               # Variable shadowing
    -Wnon-virtual-dtor     # Polymorphic class without virtual dtor
    -Wold-style-cast       # C-style casts
    -Wcast-align           # Potentially unsafe pointer alignment casts
    -Wunused               # Unused variables/functions/etc.
    -Woverloaded-virtual   # Hidden virtual overloads
    -Wnull-dereference     # Possible null dereference
    -Wdouble-promotion     # Float -> double promotion
    -Wformat=2             # Strict printf/format checking
    -Wimplicit-fallthrough # Missing [[fallthrough]]
    -Wduplicated-cond      # Duplicate conditions in if/else-if chains
    -Wduplicated-branches  # Identical bodies in conditional branches
    -Wlogical-op           # Suspicious logical operations
    -Wuseless-cast         # Redundant casts
    -Wextra-semi           # Redundant semicolons

    # Disabled warnings
    -Wno-missing-field-initializers
)

target_compile_options(inkblot_warnings INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${CLANG_GCC_WARNINGS}>
)

if(INKBLOT_WARNINGS_AS_ERRORS)
    target_compile_options(inkblot_warnings INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
    )
endif()