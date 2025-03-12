add_library(ButtonManager)
target_sources(ButtonManager
        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES ButtonManager.cppm)

add_library(Button)
target_sources(Button
        PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES Button.cppm)

# For MSVC
if(MSVC)
    target_compile_options(ButtonManager PRIVATE /interface)
    target_compile_options(Button PRIVATE /interface)
endif()

find_package(fmt CONFIG REQUIRED)
target_link_libraries(ButtonManager PRIVATE fmt::fmt)
target_link_libraries(ButtonManager PRIVATE Button)