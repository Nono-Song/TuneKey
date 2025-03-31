cmake_minimum_required(VERSION 3.30)
project(TuneKey)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if (MSVC)
    if (NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)    # parallelize each target, unless Ninja is the generator
    endif ()
    set(GCC_COVERAGE_COMPILE_FLAGS "/W4 /Zi /MDd /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
elseif (CMAKE_CXX_PLATFORM_ID STREQUAL "Linux")
    set(DBoost_DIR "home/noelucille/boost_1_87_0/stage/lib/cmake/Boost-1.87_0")
    set(GCC_COVERAGE_COMPILE_FLAGS
            "-Wall -Wextra -Wnrvo -Wpedantic -pedantic-errors -fdiagnostics-all-candidates")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
elseif (CMAKE_CXX_PLATFORM_ID STREQUAL "MinGW")
    set(GCC_COVERAGE_COMPILE_FLAGS
            "-Wall -Wextra -Wnrvo -Wpedantic -pedantic-errors -fdiagnostics-all-candidates")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

# External Libraries
find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS filesystem system)

# Main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE
        include/ButtonManager
        include/AudioController
)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ButtonManager
        AudioController
        fmt::fmt
)

# Libraries
# EventQueue
add_library(EventQueue INTERFACE)
target_include_directories(EventQueue INTERFACE include/EventQueue)

# ButtonManager
add_library(ButtonManager include/ButtonManager/ButtonManager.cpp include/ButtonManager/ButtonManager.h)
target_include_directories(ButtonManager
        PRIVATE
        include/Button
        include/AudioController)
target_link_libraries(ButtonManager
        PRIVATE
        Button
        AudioController
        Boost::filesystem
        Boost::system)

# Button
add_library(Button include/Button/Button.cpp include/Button/Button.h)
target_include_directories(Button
        PRIVATE
        include/ButtonManager
        include/AudioController)
target_link_libraries(Button
        PRIVATE
        ButtonManager
        AudioController
        PRIVATE
        Boost::filesystem
        Boost::system)

# AudioController
add_library(AudioController include/AudioController/AudioController.cpp include/AudioController/AudioController.h)
target_link_libraries(AudioController
        PUBLIC
        EventQueue
        PRIVATE
        fmt::fmt
        Boost::filesystem
        Boost::system
        SDL3::SDL3
)
