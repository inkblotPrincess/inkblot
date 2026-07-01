add_executable(inkapp)

add_subdirectory(source)

target_link_libraries(inkapp
    PRIVATE
        inkblot::inkblot
        inkblot::options
)