cmake_minimum_required(VERSION 3.28)
project(darling C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(APPLE)
    enable_language(OBJC)
    set(CMAKE_OBJC_STANDARD 23)
    set(CMAKE_OBJC_STANDARD_REQUIRED ON)
endif()

# Release: dead-code stripping so unused symbols cost zero bytes in artifacts.
# (Duplicated per repo for standalone builds; harmless when built in-tree.)
if(APPLE)
    add_compile_options($<$<CONFIG:Release>:-ffunction-sections> $<$<CONFIG:Release>:-fdata-sections>)
    add_link_options($<$<CONFIG:Release>:-Wl,-dead_strip>)
elseif(UNIX)
    add_compile_options($<$<CONFIG:Release>:-ffunction-sections> $<$<CONFIG:Release>:-fdata-sections>)
    add_link_options($<$<CONFIG:Release>:-Wl,--gc-sections>)
endif()

# Check upstream dependencies (if not already provided by parent build)
if(NOT TARGET vexspoke OR NOT TARGET hotcwap OR NOT TARGET graphvex)
    option(DARLING_USE_UPSTREAM "Depend on vexspoke + hotcwap + graphvex via FetchContent" OFF)
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
        if(NOT TARGET graphvex)
            FetchContent_Declare(
                graphvex
                GIT_REPOSITORY https://github.com/vexgraph-dev/graphvex.git
                GIT_TAG main
            )
        endif()
        FetchContent_MakeAvailable(vexspoke hotcwap graphvex)
    endif()
endif()

if(NOT TARGET darling)
add_library(darling STATIC
        c23/add.c
    darling/scene/canvas.c
    darling/compositor.c
    darling/container.c
    darling/anim/anim.c
    darling/label/label.c
    darling/panel/panel.c
    darling/picture/picture.c
    darling/label/rich_label.c
    darling/scene/scene.c
    darling/panel/layeredpanel.c
    darling/panel/sectionpanel.c
    darling/button/button.c
    darling/button/switch.c
    darling/field/checkbox.c
    darling/field/radiogroup.c
    darling/field/slider.c
    darling/field/knob.c
    darling/field/input.c
    darling/field/textarea.c
    darling/field/inputotp.c
    darling/field/select.c
    darling/field/datepicker.c
    darling/field/colorpicker.c
    darling/field/colorswatch.c
    darling/overlay/filedialog.c
    darling/label/typography.c
    darling/label/kbd.c
    darling/plot/plot.c
    event/pointer.c
    event/keyevent.c
    event/focus.c
    event/action.c
    event/value.c
    event/tree.c
    event/gesture.c
    event/dispatch.c
    event/bridge.c
    text/rich_text.c
    render/raster.c
    render/surface.c
    io/bake.c
    io/mmap.c
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
if(TARGET graphvex)
    target_link_libraries(darling PUBLIC graphvex)
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
