cmake_minimum_required(VERSION 3.5)

# set(CMAKE_C_STANDARD 90)

project(flappybird)

# Create an option to switch between a system sdl library and a vendored sdl library
option(FLAPPYBIRD_VENDORED "Use vendored libraries" OFF)

if(FLAPPYBIRD_VENDORED)
    add_subdirectory(vendored/sdl EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

    # 1. Look for a SDL2 package, 2. Look for the SDL2maincomponent and 3. DO NOT fail when SDL2main is not available
    find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
endif()

# Create your game executable target as usual
add_executable(flappybird main.c)

set_property(TARGET flappybird PROPERTY COMPILE_WARNING_AS_ERROR ON)

target_include_directories(flappybird PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_mixer_INCLUDE_DIRS})

message(STATUS "SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
message(STATUS "SDL2_MIXER_INCLUDE_DIRS: ${SDL2_MIXER_INCLUDE_DIRS}")

# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(flappybird PRIVATE SDL2::SDL2main)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(flappybird PRIVATE SDL2::SDL2)