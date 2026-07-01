add_library(inkblot STATIC)
add_library(inkblot::inkblot ALIAS inkblot)

add_subdirectory(source)

target_compile_features(inkblot PUBLIC cxx_std_23)

target_include_directories(inkblot 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>    
)

target_link_libraries(inkblot
    PRIVATE
        inkblot::options
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(inkblot PUBLIC stdc++exp)
endif()