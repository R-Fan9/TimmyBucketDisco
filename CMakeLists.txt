cmake_minimum_required(VERSION 3.26)
project(FacialExps)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_FILES main.cpp glad.c)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/asset DESTINATION ${CMAKE_BINARY_DIR})

add_executable(FacialExps ${SOURCE_FILES})

target_link_libraries(FacialExps glfw)
