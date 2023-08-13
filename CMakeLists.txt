cmake_minimum_required(VERSION 3.20)

project(LiveMod)

add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(src)

add_subdirectory(tests)
