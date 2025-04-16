# AudioController
add_library(AudioController STATIC AudioController.cpp AudioController.hpp)
target_include_directories(AudioController PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AudioController PUBLIC Event)