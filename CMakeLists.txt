cmake_minimum_required(VERSION 3.28)
project(darling C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(APPLE)
    enable_language(OBJC)
    set(CMAKE_OBJC_STANDARD 23)
    set(CMAKE_OBJC_STANDARD_REQUIRED ON)
endif()

# Check upstream dependencies (if not already provided by parent build)
if(NOT TARGET vexspoke OR NOT TARGET hotcwap)
    option(DARLING_USE_UPSTREAM "Depend on vexspoke + hotcwap via FetchContent" OFF)
    if(DARLING_USE_UPSTREAM)
        include(FetchContent)
        if(NOT TARGET vexspoke)
            FetchContent_Declare(
                vexspoke
                GIT_REPOSITORY https://github.com/vexgraph-dev/vexspoke.git
                GIT_TAG main
            )
        endif()
        if(NOT TARGET hotcwap)
            FetchContent_Declare(
                hotcwap
                GIT_REPOSITORY https://github.com/vexgraph-dev/hotcwap.git
                GIT_TAG main
            )
        endif()
        FetchContent_MakeAvailable(vexspoke hotcwap)
    endif()
endif()

if(NOT TARGET darling)
add_library(darling STATIC
    darling/canvas.c
    darling/compositor.c
    darling/container.c
    darling/label.c
    darling/panel.c
    darling/picture.c
    darling/rich_label.c
    darling/scene.c
    font/font.c
    font/font_bake.c
    text/rich_text.c
    render/raster.c
    render/surface.c
    io/bake.c
    io/vfs.c
    io/mmap.c
    vulkan/vk_scene.c
    vulkan/vk_view.c
    vulkan/vk_iosurface.c
    vulkan/sdf_gpu.c
    vulkan/texture/texture.c
    window/panel_bridge.c
)

target_include_directories(darling PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(darling PRIVATE -Wall -Wextra -Werror -mcpu=native)

find_package(Vulkan QUIET)
if(Vulkan_FOUND)
    target_include_directories(darling PUBLIC ${Vulkan_INCLUDE_DIRS})
elseif(EXISTS "/opt/homebrew/include/vulkan/vulkan.h")
    target_include_directories(darling PUBLIC /opt/homebrew/include)
endif()

if(TARGET vexspoke)
    target_link_libraries(darling PUBLIC vexspoke)
endif()
if(TARGET hotcwap)
    target_link_libraries(darling PUBLIC hotcwap)
endif()

# Cocoa UI shims (macOS only): panel layers, CoreText raster, font lookup.
if(APPLE)
    target_sources(darling PRIVATE
        objc/panel_cocoa.m
        objc/text_core.m
        objc/font_cocoa.m
    )
    set_source_files_properties(
        objc/panel_cocoa.m
        objc/text_core.m
        objc/font_cocoa.m
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
    )
    target_link_libraries(darling PUBLIC
        "-framework Cocoa"
        "-framework AppKit"
        "-framework QuartzCore"
        "-framework CoreGraphics"
        "-framework IOSurface"
    )
else()
    target_sources(darling PRIVATE text/text_core_stub.c)
endif() # APPLE
endif() # NOT TARGET darling

if(NOT TARGET darling-ui)
    add_library(darling-ui ALIAS darling)
endif()
if(NOT TARGET darling::ui)
    add_library(darling::ui ALIAS darling)
endif()
