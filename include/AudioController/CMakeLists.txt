# AudioController
add_library(AudioController STATIC IAudioController.cpp IAudioController.hpp)
target_include_directories(AudioController PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AudioController PUBLIC Event)