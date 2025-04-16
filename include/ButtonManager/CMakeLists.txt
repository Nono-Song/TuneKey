# ButtonManager
add_library(ButtonManager ButtonManager.cpp ButtonManager.hpp)
target_include_directories(ButtonManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ButtonManager
        PRIVATE
        AudioButton
        AudioControllerFactory)