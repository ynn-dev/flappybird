cmake_minimum_required(VERSION 3.5)
project(flappybird C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

# Create your game executable target as usual
add_executable(flappybird flappybird.c)

set_property(TARGET flappybird PROPERTY COMPILE_WARNING_AS_ERROR ON)

target_link_libraries(flappybird
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
target_link_libraries(flappybird PRIVATE $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>)
target_link_libraries(flappybird PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)

# Define _CRT_SECURE_NO_WARNINGS globally
if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()