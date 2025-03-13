cmake_minimum_required(VERSION 3.30)
project(TuneKey)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
else ()
    set(DBoost_DIR "home/noelucille/boost_1_87_0/stage/lib/cmake/Boost-1.87_0")
    set(GCC_COVERAGE_COMPILE_FLAGS
            "-Wall -Wextra -Wpedantic -pedantic-errors -Wpessimizing-move -fdiagnostics-all-candidates")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

# External Libraries
find_package(fmt CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem system)

# Main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE
        include/ButtonManager
)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ButtonManager
        Button)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        fmt::fmt)

# Libraries
# EventQueue
add_library(EventQueue include/EventQueue/EventQueue.h include/EventQueue/EventQueue.cpp)
target_include_directories(EventQueue
        PRIVATE
        include/Button)
target_link_libraries(EventQueue
        PRIVATE
        Button)

# ButtonManager
add_library(ButtonManager include/ButtonManager/ButtonManager.cpp include/ButtonManager/ButtonManager.h)
target_include_directories(ButtonManager
        PUBLIC
        include/Button
        include/EventQueue)
target_link_libraries(ButtonManager
        PRIVATE
        Button
        EventQueue)
target_link_libraries(ButtonManager
        PRIVATE
        fmt::fmt)

# Button
add_library(Button include/Button/Button.cpp include/Button/Button.h)
target_link_libraries(Button
        PUBLIC
        Boost::filesystem
        Boost::system)



