# Project info
cmake_minimum_required(VERSION "3.2.2" FATAL_ERROR)
project("Nito")
set(CMAKE_PATH "$ENV{CMAKE_PATH}")

# Utilities
include("${CMAKE_PATH}/macros/configure_project.cmake")
include("${CMAKE_PATH}/macros/configure_magick_compiler_options.cmake")
include("${CMAKE_PATH}/macros/module_dependency.cmake")
include("${CMAKE_PATH}/macros/add_lib.cmake")
include("${CMAKE_PATH}/macros/test_module_dependency.cmake")
include("${CMAKE_PATH}/macros/add_lib_tests.cmake")

# Configuration
configure_project()
configure_magick_compiler_options()

# Library
module_dependency("GLEW")
module_dependency("GLFW3")
module_dependency("OpenAL")
module_dependency("ALUT")
module_dependency("Magick++")
module_dependency("FreeType2")
module_dependency("Cpp_Utils")
add_lib(SHARED)

# Tests
test_module_dependency("GoogleTest")
add_lib_tests()
