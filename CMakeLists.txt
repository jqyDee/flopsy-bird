cmake_minimum_required(VERSION 3.10)

project(flopsy-bird)
set(SOURCES
    src/main.c
)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(SDL_DIR ${LIB_DIR}/SDL2)
set(SDL_IMAGE_DIR ${LIB_DIR}/SDL2_image)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(${SDL_DIR})
add_subdirectory(${SDL_IMAGE_DIR})

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 SDL2_image)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

add_compile_options(-Wall -Wextra)
add_link_options(-Wall -Wextra)
