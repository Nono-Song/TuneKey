cmake_minimum_required(VERSION 3.30)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# prevent installing to system directories.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

project(TuneKey)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if ((APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin") OR EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")    # Disable shared builds on platforms where it does not make sense to use them
    set(SDL_SHARED OFF)
else()
    set(SDL_SHARED ON)
endif()

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)				# parallelize each target, unless Ninja is the generator
    endif()
endif()


if (MSVC)
    set(GCC_COVERAGE_COMPILE_FLAGS "/W4 /Zi /MDd /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -Wpedantic -pedantic-errors -fexperimental-library -fsanitize=address,undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
elseif (CMAKE_CXX_PLATFORM_ID STREQUAL "Linux")
    set(DBoost_DIR "home/noelucille/boost_1_87_0/stage/lib/cmake/Boost-1.87_0")
    set(GCC_COVERAGE_COMPILE_FLAGS
            "-Wall -Wextra -Wnrvo -Wpedantic -pedantic-errors -fdiagnostics-all-candidates")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(GCC_COVERAGE_COMPILE_FLAGS
            "-Wall -Wextra -Wnrvo -Wpedantic -pedantic-errors")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

# External Libraries
find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
# SDL_mixer (used for playing audio)
set(SDLMIXER_MIDI_NATIVE OFF)     # disable formats we don't use to make the build faster and smaller. Also some of these don't work on all platforms so you'll need to do some experimentation.
set(SDLMIXER_GME OFF)
set(SDLMIXER_WAVPACK OFF)
set(SDLMIXER_MOD OFF)
set(SDLMIXER_OPUS OFF)
set(SDLMIXER_VENDORED ON)   # tell SDL_mixer to build its own dependencies
add_subdirectory(SDL_mixer EXCLUDE_FROM_ALL)
add_subdirectory(include/IAudioController)
add_subdirectory(include/AudioControllerImpl)
add_subdirectory(include/AudioControllerFactory)
add_subdirectory(include/EventQueue)
add_subdirectory(include/Button)
add_subdirectory(include/ButtonManager)

# Main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ButtonManager
        SDL3::SDL3
        fmt::fmt
)
