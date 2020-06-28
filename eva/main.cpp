#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>

#include <iostream>
#include <random>
#include <cmath>
#include <ctime>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


struct _concTextures genConcTextures();
void initConcTextures(struct _concTextures concTextures);

bool randomize_pending = false;

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation

    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    int window_width = mode->width;
    int window_height = mode->height;
    
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "ahhhhh!", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader programs and texure
    Shader ourShader("shader.vert", "shader.frag"); 

    glEnable(GL_DEPTH_TEST);

    float vertices[] = {
        -1.0f,  1.0f, 0.0f,     0.0f, 1.0f, 
         1.0f,  1.0f, 0.0f,     1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f
    };
    unsigned int vertices_indicies[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertices_indicies), vertices_indicies, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    srand((unsigned)time(NULL));

    // render loop
    clock_t begin = clock();
    clock_t accumilator = clock();
    int time_step = 300;
    float phase = 0;
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        ourShader.use();
        glBindVertexArray(VAO);
        glUniform1f(glGetUniformLocation(ourShader.ID, "phase"), phase);
        float aspect_ratio = (float)window_width / (float)window_height;
        glUniform1f(glGetUniformLocation(ourShader.ID, "aspect_ratio"), aspect_ratio);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (clock() - accumilator > time_step) {
            accumilator += time_step;
            phase -= 1;
        }
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
