cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(chip8 VERSION 0.1 LANGUAGES C)

include(GNUInstallDirs)

find_package(sdl2 REQUIRED)

add_subdirectory(chip8)
