add_library(inkblot_options INTERFACE)
add_library(inkblot::options ALIAS inkblot_options)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(inkblot_options 
        INTERFACE
            -fcontract-evaluation-semantic=enforce
            -freflection
    )
else()
    message(FATAL_ERROR "This project requires C++26, and only GCC supports contracts and reflection")
endif()

target_compile_features(inkblot_options
    INTERFACE
        cxx_std_26
)

set_target_properties(inkblot_options 
    PROPERTIES
        CXX_EXTENSIONS OFF
)

target_link_libraries(inkblot_options
    INTERFACE
        inkblot_warnings
        inkblot_sanitizers
)