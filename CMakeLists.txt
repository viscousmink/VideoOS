# ==============================================================================
# VideoOS CMake Build System for Raspberry Pi 4 Development
# ==============================================================================
# This CMakeLists.txt provides an alternative to the Makefile build system
# for cross-platform development on Windows, Linux, and macOS.
# ==============================================================================

cmake_minimum_required(VERSION 3.15)
project(VideoOS VERSION 1.0.0 LANGUAGES C ASM)

# =============================================================================
# Platform Detection
# =============================================================================
set(RPI_PLATFORM "ARM64")
if(NOT WIN32)
    set(RPI_ARCH "native")
else()
    set(RPI_ARCH "cross_compile")
endif()

message(STATUS "Platform: ${RPI_PLATFORM}")
message(STATUS "Architecture: ${RPI_ARCH}")

# =============================================================================
# Compiler Flags for ARM64 (Raspberry Pi 4)
# =============================================================================

# C Compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    if(APPLE)
        # macOS/Apple Silicon
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target arm64-apple-darwin")
    elseif(RPI_ARCH STREQUAL "cross_compile")
        # Cross-compilation for Raspberry Pi
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+crc+crypto -O2 -Wall -Wextra")
    else()
        # Native ARM64 build (Linux/Windows on ARM)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -O2 -Wall -Wextra")
    endif()
endif()

# C++ Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target arm64-apple-darwin")
    elseif(RPI_ARCH STREQUAL "cross_compile")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+crc+crypto -O2 -Wall -Wextra")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O2 -Wall -Wextra")
    endif()
endif()

# Assembly Compiler flags (GNU Assembler)
set(ASM_CFLAGS "-march=armv8-a+crc+crypto -gdwarf-4")
if(NOT WIN32)
    set(ASM_CFLAGS "${ASM_CFLAGS} -Wa,-ahlms=aarch64")
endif()

# =============================================================================
# Find FFmpeg Libraries (optional)
# =============================================================================
option(FIND_FFMPEG "Search for FFmpeg libraries" ON)
if(FIND_FFMPEG AND NOT WIN32)
    find_package(PkgConfig REQUIRED)
    
    pkg_check_modules(FFMPEG_CORE
        IMPORTED GLOBAL
        REQUIRED
        libavformat
        libavcodec
        libavutil
        libswscale
        libswresample
    )
    
    message(STATUS "Found FFmpeg:")
    message(STATUS "  Include dirs: ${FFMPEG_CORE_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${FFMPEG_CORE_LIBRARIES}")
endif()

# =============================================================================
# Source Files
# =============================================================================

# Assembly sources
set(ASM_SOURCES
    src/assembly/start.s
)

# C sources
set(C_SOURCES
    src/c/core.c
    src/c/handlers.c
    tools/circle/circle_core.c
)

# C++ sources
set(CPP_SOURCES
    src/cpp/video_processor.cpp
    src/cpp/main.cpp
)

# =============================================================================
# Executable Target
# =============================================================================

add_executable(videoos_boot
    ${ASM_SOURCES}
    ${C_SOURCES}
    ${CPP_SOURCES}
)

# Link libraries
if(FIND_FFMPEG)
    target_link_libraries(videoos_boot PRIVATE
        ${FFMPEG_CORE_LIBRARIES}
    )
    
    # Include directories for FFmpeg
    include_directories(${FFMPEG_CORE_INCLUDE_DIRS})
endif()

# =============================================================================
# Compile Definitions
# =============================================================================

target_compile_definitions(videoos_boot PRIVATE
    $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
    $<$<CONFIG:Debug>:DEBUG_MODE>
)

# =============================================================================
# Additional Targets
# =============================================================================

# Debug build
add_executable(videoos_debug
    ${ASM_SOURCES}
    ${C_SOURCES}
    ${CPP_SOURCES}
    -DCMAKE_BUILD_TYPE=Debug
)

target_compile_options(videoos_debug PRIVATE
    $<$<CONFIG:Debug>:-g3;-O0;-fsanitize=address>
)

# Release build
add_executable(videoos_release
    ${ASM_SOURCES}
    ${C_SOURCES}
    ${CPP_SOURCES}
    -DCMAKE_BUILD_TYPE=Release
)

target_compile_options(videoos_release PRIVATE
    $<$<CONFIG:Release>:-O3;-DNDEBUG>
)

# =============================================================================
# Circle Library (Static)
# =============================================================================

add_library(circle_core STATIC
    tools/circle/circle_core.c
)

target_include_directories(circle_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# =============================================================================
# Assembly-only Target (for testing assembly files)
# =============================================================================

add_custom_target(asm_only
    COMMAND ${ASM_COMPILER} ${ASM_CFLAGS} -c -o obj/start.o src/assembly/start.s
    COMMENT "Building assembly object file..."
    VERBATIM
)

# =============================================================================
# Installation Rules
# =============================================================================

install(TARGETS videoos_boot
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include/videoos
    FILES_MATCHING PATTERN "*.h"
)

# =============================================================================
# Documentation (optional)
# =============================================================================

option(BUILD_DOCS "Build documentation with Doxygen" OFF)
if(BUILD_DOCS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile")
    add_custom_target(docs
        COMMAND doxygen Doxyfile
        COMMENT "Generating API documentation..."
    )
endif()

# =============================================================================
# Help Target
# =============================================================================

configure_file(README.md ${CMAKE_BINARY_DIR}/README.md COPYONLY)

# =============================================================================
# End of CMakeLists.txt
# =============================================================================