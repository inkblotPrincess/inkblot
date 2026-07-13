target_sources(inkblot PRIVATE
    vulkan_context.cpp
    vulkan_renderer.cpp
)

if(WIN32)
    target_sources(inkblot PRIVATE
        vulkan_platform_win32.cpp
    )
else()
    message(FATAL_ERROR "Platform unrecognised")
endif()