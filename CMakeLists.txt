cmake_minimum_required(VERSION 3.30)
project(TuneKey)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(GCC_COVERAGE_COMPILE_FLAGS "-Wall")
else()
    set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -Wpedantic -Wpessimizing-move  -fdiagnostics-all-candidates")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")


# Add the library as a subdirectory
add_subdirectory(src/Button)

# Main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ButtonManager)


find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
