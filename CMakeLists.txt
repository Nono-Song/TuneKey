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
        include/Button
        include/EventQueue
)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Button
        EventQueue
        ButtonManager
        AudioController
        Event
        SDL3::SDL3
        fmt::fmt
)

# Libraries
# EventQueue
add_library(EventQueue INTERFACE)
target_include_directories(EventQueue INTERFACE include/EventQueue)

add_library(Event INTERFACE)
target_include_directories(Event INTERFACE include/EventQueue)
target_link_libraries(Event INTERFACE Boost::filesystem Boost::system)

# ButtonManager
add_library(ButtonManager include/ButtonManager/ButtonManager.cpp include/ButtonManager/ButtonManager.h)
target_include_directories(ButtonManager
        PRIVATE
        include/Button
        include/AudioController
        include/Event)
target_link_libraries(ButtonManager
        PRIVATE
        Button
        Event
        AudioController
        Boost::filesystem
        Boost::system)

# Button
add_library(Button include/Button/Button.hpp include/Button/Button.cpp)
target_include_directories(Button
        PRIVATE
        include/AudioController
        include/EventQueue)
target_link_libraries(Button
        PUBLIC
        Event
        AudioController
        PRIVATE
        Boost::filesystem
        Boost::system)

#AudioButton
add_library(AudioButton INTERFACE include/Button/SpecialButtons.hpp)
target_link_libraries(AudioButton INTERFACE Button)


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