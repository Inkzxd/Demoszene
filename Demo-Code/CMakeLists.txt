cmake_minimum_required(VERSION 3.10)
project(RetroTerminal)

set(CMAKE_CXX_STANDARD 17)

# set header files path
include_directories(
    include
    C:/msys64/mingw64/include
    C:/msys64/mingw64/include/GLFW
    C:/msys64/mingw64/include/freetype2
    C:/msys64/mingw64/include/glm
    C:/msys64/mingw64/include/SFML
    ${CMAKE_SOURCE_DIR}/src
)

# set library path
link_directories(
    C:/msys64/mingw64/lib
)

# source files
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.c
)

# executable file
add_executable(RetroTerminal ${SOURCES})

set_target_properties(RetroTerminal PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

configure_file(
    ${CMAKE_SOURCE_DIR}/config.ini
    ${CMAKE_BINARY_DIR}/config.ini
    COPYONLY
)


# link libraries
target_link_libraries(RetroTerminal
    glfw3
    freetype
    opengl32
    gdi32
    user32
    kernel32
    sfml-audio
    sfml-system
)

message(STATUS "Link directories: ${CMAKE_LIBRARY_PATH}")