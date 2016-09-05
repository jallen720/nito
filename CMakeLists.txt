# Project info
cmake_minimum_required(VERSION "3.2.2" FATAL_ERROR)
project("Nito")
set(CMAKE_PATH "$ENV{CMAKE_PATH}")

# Utilities
include("${CMAKE_PATH}/macros/configure_project.cmake")
include("${CMAKE_PATH}/macros/configure_magick_compiler_options.cmake")
include("${CMAKE_PATH}/macros/module_dependency.cmake")
include("${CMAKE_PATH}/macros/add_bin.cmake")
include("${CMAKE_PATH}/macros/test_module_dependency.cmake")
include("${CMAKE_PATH}/macros/add_bin_tests.cmake")

# Configuration
configure_project()
configure_magick_compiler_options()

# App binary
module_dependency("CppUtils")
module_dependency("GLFW3")
module_dependency("GLEW")
module_dependency("Magick++")
add_bin()

# Tests
test_module_dependency("GoogleTest")
add_bin_tests()
