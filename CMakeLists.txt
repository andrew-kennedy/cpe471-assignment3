cmake_minimum_required(VERSION 2.8)

# Name of the project
project(assignment3)

set(CMAKE_CXX_STANDARD 14)

# Use glob to get the list of all source files.
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Set the executable.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS})

find_package(GLM REQUIRED)

find_package(assimp REQUIRED)

find_package(SFML REQUIRED COMPONENTS window system)

find_package(glbinding REQUIRED HINTS /usr/local/opt/glbinding)

find_package(globjects REQUIRED HINTS /usr/local/opt/globjects)

find_package(devil REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} il ilu ilut assimp glbinding::glbinding globjects::globjects ${SFML_LIBRARIES})

# OS specific options and libraries
if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic")
    if (APPLE)
        # Add required frameworks for GLFW.
        target_link_libraries(${CMAKE_PROJECT_NAME} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
    else ()
        #Link the Linux OpenGL library
        target_link_libraries(${CMAKE_PROJECT_NAME} "GL")
    endif ()
endif ()
