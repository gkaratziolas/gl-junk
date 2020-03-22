cmake_minimum_required(VERSION 2.8)
project(truchet)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")

add_subdirectory(../lib/glfw-3.3.2 lib)
include_directories(../lib/glfw-3.3.2/include)
include_directories(../lib/glad/include)
include_directories(../lib/glm/)

set(GLAD_SRC ../lib/glad/src/glad.c)

add_executable("${PROJECT_NAME}"
        "truchet.cpp"
        ${GLAD_SRC})

target_link_libraries(truchet glfw ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${GLAD_LIBRARIES} ${GLFW_LIBRARIES})
