# External Libraries
find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

add_library(SDLManager INTERFACE SDLManager.hpp)
target_include_directories(SDLManager INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SDLManager INTERFACE SDL3_mixer::SDL3_mixer)

add_library(AudioControllerImpl
        STATIC
        AudioControllerImpl.cpp
        AudioControllerImpl.hpp)
target_link_libraries(AudioControllerImpl PUBLIC IAudioController PRIVATE fmt::fmt SDLManager)
target_include_directories(AudioControllerImpl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})