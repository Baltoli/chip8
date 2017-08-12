cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(chip8 VERSION 0.1 LANGUAGES C)

include(GNUInstallDirs)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(chip8)
