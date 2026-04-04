cmake_minimum_required(VERSION 3.16)

project(PacManING2 VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(PACMAN_ENABLE_WARNINGS "Enable extra compiler warnings" ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(Projectp
    main.c
    button.c
)

if(WIN32)
    target_sources(Projectp PRIVATE Projectp_private.rc)
endif()

target_include_directories(Projectp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Projectp PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
    SDL2_ttf::SDL2_ttf
)

if(PACMAN_ENABLE_WARNINGS)
    target_compile_options(Projectp PRIVATE
        $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
        $<$<C_COMPILER_ID:MSVC>:/W4>
    )
endif()

if(WIN32)
    target_link_options(Projectp PRIVATE -mwindows)
endif()
