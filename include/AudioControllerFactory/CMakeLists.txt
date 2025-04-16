add_library(AudioControllerFactory STATIC AudioControllerFactory.hpp AudioControllerFactory.cpp)
target_include_directories(AudioControllerFactory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AudioControllerFactory
        PUBLIC
        AudioController
        PRIVATE
        AudioControllerImpl)