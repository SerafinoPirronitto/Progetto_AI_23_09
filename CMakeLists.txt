cmake_minimum_required(VERSION 3.10.0)
project(Progetto_AI_23_09 VERSION 0.1.0 LANGUAGES C CXX)

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/std:c++23)
else()
    add_compile_options(-std=c++23)
endif()

add_executable(Progetto_AI_23_09 maincpp.cpp AntColony.cpp Parser.cpp)

include(CTest)
enable_testing()

