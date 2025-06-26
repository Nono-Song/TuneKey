add_library(EventQueue INTERFACE)
target_include_directories(EventQueue INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Event INTERFACE)
target_link_libraries(Event INTERFACE Typedefs)
target_include_directories(Event INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})