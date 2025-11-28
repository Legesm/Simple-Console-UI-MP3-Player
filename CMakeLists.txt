cmake_minimum_required(VERSION 3.31)
project(PlayerTest1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("C:/C++/Project/PlayerTest1/src/")

set(PORTAUDIO_LIB "C:/C++/Project/PlayerTest1/src/portaudio.lib")

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v6.1.9
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(ftxui)

add_executable(PlayerTest1 src/main.cpp
        src/player.cpp
        src/player.h)
target_compile_definitions(PlayerTest1 PRIVATE NOMINMAX)


target_link_libraries(PlayerTest1 PRIVATE
        # Chose a submodule
        ftxui::component
        ftxui::dom
        ftxui::screen
        ${PORTAUDIO_LIB}
)




