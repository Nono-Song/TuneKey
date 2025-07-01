# Button
add_library(Button STATIC Button.hpp Button.cpp)
target_include_directories(Button PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Button PRIVATE IAudioController)

#AudioButton
add_library(AudioButton STATIC AudioButtons.hpp AudioButtons.cpp)
target_include_directories(AudioButton PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AudioButton PUBLIC Button PRIVATE IAudioController)