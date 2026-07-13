add_subdirectory(inkblot)

if(INKBLOT_BUILD_APP)
    add_subdirectory(app)
endif()

if(INKBLOT_BUILD_TESTS)
    add_subdirectory(tests)
endif()