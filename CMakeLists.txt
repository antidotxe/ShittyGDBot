cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

project(GdBot VERSION 0.1.0 LANGUAGES CXX)

set(PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Core/ModBootstrap.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Cosmetics/CosmeticUnlockService.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Macro/BotController.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Macro/InputRecorder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Macro/MacroSessionCoordinator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Features/Macro/ReplayController.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hooks/CosmeticUnlockBypassHook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hooks/FramePresentationHook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hooks/KeyboardInputHook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hooks/MenuLayerHook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hooks/OverlayInputBlockHook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hooks/OverlayLifecycleHook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hooks/PlayLayerSessionHook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hooks/PlayerInputHook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Hooks/ReplayDispatchHook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Overlay/ImGuiOverlayBackend.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Overlay/ImGuiOpenGlBackend.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Overlay/ImGuiOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Overlay/OverlayInputRouter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Overlay/OverlayTheme.cpp"
)

set(IMGUI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_win32.cpp"
)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends"
)
target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK. Set the GEODE_SDK environment variable first.")
endif()

message(STATUS "Found Geode SDK at $ENV{GEODE_SDK}")
add_subdirectory($ENV{GEODE_SDK} "${CMAKE_CURRENT_BINARY_DIR}/geode")

setup_geode_mod(${PROJECT_NAME})
