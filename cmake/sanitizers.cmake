add_library(inkblot_sanitizers INTERFACE)

option(INKBLOT_ENABLE_ASAN  "Enable AddressSanitizer"           OFF)
option(INKBLOT_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(INKBLOT_ENABLE_TSAN  "Enable ThreadSanitizer"            OFF)

if(INKBLOT_ENABLE_ASAN AND INKBLOT_ENABLE_TSAN)
    message(FATAL_ERROR "ASan and TSan cannot be enabled at the same time.")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    if(INKBLOT_ENABLE_ASAN)
        target_compile_options(inkblot_sanitizers INTERFACE
            -fsanitize=address
            -fno-omit-frame-pointer
        )

        target_link_options(inkblot_sanitizers INTERFACE
            -fsanitize=address
        )
    endif()

    if(INKBLOT_ENABLE_UBSAN)
        target_compile_options(inkblot_sanitizers INTERFACE
            -fsanitize=undefined
            -fno-omit-frame-pointer
        )

        target_link_options(inkblot_sanitizers INTERFACE
            -fsanitize=undefined
        )
    endif()

    if(INKBLOT_ENABLE_TSAN)
        target_compile_options(inkblot_sanitizers INTERFACE
            -fsanitize=thread
            -fno-omit-frame-pointer
        )

        target_link_options(inkblot_sanitizers INTERFACE
            -fsanitize=thread
        )
    endif()
elseif(MSVC)
    if(INKBLOT_ENABLE_ASAN)
        target_compile_options(inkblot_sanitizers INTERFACE
            /fsanitize=address
        )

        target_link_options(inkblot_sanitizers INTERFACE
            /INCREMENTAL:NO
        )
    endif()
endif()