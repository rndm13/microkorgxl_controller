cmake_minimum_required(VERSION 3.12)
project(microkorgxl_controller)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (!WIN32)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Wunused -Wconversion -Wuseless-cast -Wnull-dereference -Wlogical-op $<$<CONFIG:DEBUG>:-fsanitize=address,undefined,leak>")
else ()
    set(CMAKE_CXX_FLAGS "")
endif ()

# Add imgui_bundle
# =================
# 1/  Option 1: if you added imgui_bundle in a subfolder, you can add it to your project with:
if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui_bundle)
    add_subdirectory(external/imgui_bundle)
endif()

## 2/  Option 2: simply fetch imgui_bundle during the build
if (NOT TARGET imgui_bundle)
    message(STATUS "Fetching imgui_bundle...")
    include(FetchContent)
    Set(FETCHCONTENT_QUIET FALSE)
    FetchContent_Declare(imgui_bundle GIT_REPOSITORY https://github.com/pthom/imgui_bundle.git GIT_TAG v1.92.0)
    FetchContent_MakeAvailable(imgui_bundle)

    # Uncomment the next line if you which to also automatically fetch and compile OpenCV for immvision support
    # set(IMMVISION_FETCH_OPENCV ON)
endif()

# Add JACK
# ==============

find_package(jack)
if (NOT jack_FOUND)
    message(FATAL_ERROR "JACK Library not found.")
endif()

# Build your app
# ==============
# imgui_bundle_add_app is a helper function, similar to cmake's "add_executable"
# Usage:
# imgui_bundle_add_app(app_name file1.cpp file2.cpp ...)
#
# Features:
# * It will automatically link the target to the required libraries (hello_imgui, OpenGl, glad, etc)
# * It will embed the assets (for desktop, mobile, and emscripten apps)
# * It will perform additional customization (app icon and name on mobile platforms, etc)
#
# Now you can build your app with
#     mkdir build && cd build && cmake .. && cmake --build .

# portable installation is the easiest way to install the app
imgui_bundle_add_app(microkorgxl_controller src/main.cpp)

include_directories(microkorgxl_controller ${JACK_INCLUDE_DIRS})
target_link_libraries(microkorgxl_controller PRIVATE ${JACK_LIBRARIES})
if (${JACK_DEFINITIONS})
    add_definitions(microkorgxl_controller ${JACK_DEFINITIONS})
endif()
