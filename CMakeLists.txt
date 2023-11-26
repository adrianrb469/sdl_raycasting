cmake_minimum_required(VERSION 3.10)

project(GAME)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")

# Use pkg-config to get the flags for SDL2 and its extensions
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
pkg_check_modules(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
pkg_check_modules(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Include the SDL2, SDL2_image and SDL2_mixer headers
target_include_directories(${PROJECT_NAME}
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include
      ${SDL2_INCLUDE_DIRS}
      ${SDL2IMAGE_INCLUDE_DIRS}
      ${SDL2MIXER_INCLUDE_DIRS}
        ${SDL2TTF_INCLUDE_DIRS}
)

# Link the SDL2, SDL2_image and SDL2_mixer libraries
target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${SDL2IMAGE_LIBRARIES}
    ${SDL2MIXER_LIBRARIES}
    ${SDL2TTF_LIBRARIES}
    box2d
)

# Link the libraries for SDL2, SDL2_image and SDL2_mixer
target_link_directories(${PROJECT_NAME}
    PRIVATE
      ${SDL2_LIBRARY_DIRS}
      ${SDL2IMAGE_LIBRARY_DIRS}
      ${SDL2MIXER_LIBRARY_DIRS}
        ${SDL2TTF_LIBRARY_DIRS}
)
