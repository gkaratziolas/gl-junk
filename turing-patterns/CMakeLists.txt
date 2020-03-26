cmake_minimum_required(VERSION 2.8)
project(turing)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")

add_subdirectory(../lib/glfw-3.3.2 lib)
include_directories(../lib/glfw-3.3.2/include)
include_directories(../lib/glad/include)
include_directories(../lib/glm/)
include_directories(.)

set(GLAD_SRC ../lib/glad/src/glad.c)

file(COPY texture.fs DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY texture.vs DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY turing.cs DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable("${PROJECT_NAME}"
        "main.cpp"
        "shader.h"
        ${GLAD_SRC})

target_link_libraries(turing glfw ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${GLAD_LIBRARIES} ${GLFW_LIBRARIES})
