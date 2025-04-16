# External Libraries
find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

add_library(AudioControllerImpl STATIC AudioControllerImpl.cpp AudioControllerImpl.hpp)
target_link_libraries(AudioControllerImpl PUBLIC AudioController fmt::fmt)
target_include_directories(AudioControllerImpl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})