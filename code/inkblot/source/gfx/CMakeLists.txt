target_sources(inkblot PRIVATE
    renderer.cpp
)

if(INKBLOT_USE_VULKAN)
    add_subdirectory(vulkan)
endif()