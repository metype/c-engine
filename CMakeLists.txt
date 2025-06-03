cmake_minimum_required(VERSION 3.26)
project(CEngine C)

set(CMAKE_C_STANDARD 23)

file(GLOB_RECURSE SOURCES src/*.c)

set(SDL_VULKAN OFF)

add_subdirectory(SDL EXCLUDE_FROM_ALL)
add_subdirectory(SDL_Image EXCLUDE_FROM_ALL)
add_subdirectory(SDL_mixer EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME} ${SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PUBLIC -g)
    add_compile_definitions(DEBUG=1)
else()
    target_compile_options(${PROJECT_NAME} PUBLIC -O3 -flto)
    target_link_options(${PROJECT_NAME} PUBLIC -O3 -flto)
    add_compile_definitions(DEBUG=0)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer m)

target_compile_options(${PROJECT_NAME} PUBLIC -Wno-format-truncation)
target_link_options(${PROJECT_NAME} PUBLIC -Wno-format-truncation)

#message(STATUS, "CFLAGS: ${CMAKE_C_FLAGS}")

#if(UNIX)
#    find_package(PkgConfig REQUIRED)
#    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
#    target_include_directories(${PROJECT_NAME} PRIVATE ${GTK3_INCLUDE_DIRS})
#    target_link_directories(${PROJECT_NAME} PRIVATE ${GTK_LIBRARY_DIRS})
#    target_link_libraries(${PROJECT_NAME} PRIVATE ${GTK3_LIBRARIES})
#    target_compile_options(${PROJECT_NAME} PRIVATE ${GTK_CFLAGS_OTHER})
#endif(UNIX)
