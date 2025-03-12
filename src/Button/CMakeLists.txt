add_library(ButtonManager ButtonManager.cpp)

add_library(Button Button.cpp)



find_package(fmt CONFIG REQUIRED)
target_link_libraries(ButtonManager PRIVATE fmt::fmt)
target_link_libraries(ButtonManager PRIVATE Button)