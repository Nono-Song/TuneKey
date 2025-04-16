# Button
add_library(Button Button.hpp Button.cpp)
target_include_directories(Button PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Button PUBLIC AudioControllerFactory)

#AudioButton
add_library(AudioButton INTERFACE SpecialButtons.hpp)
target_include_directories(AudioButton INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AudioButton INTERFACE Button)