#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>
#include <ctime>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 transform;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = transform * vec4(aPos, 1.0f);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "uniform vec3 uColour;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(uColour, 1.0f);\n"
    "}\n\0";

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ahhhhh!", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    int shaderProgram  = glCreateProgram();
    int shaderTriangle = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glEnable(GL_DEPTH_TEST);

    /* 
     * Set up vertex data for truchet tile
     *
     *              4-------5
     *             /|      /|
     *            0-------1 |
     *            | |     | |
     *            | 6-----|-7
     *            |/      |/
     *            2-------3
     */

    float tile_width = 0.1f;
    float tile_depth = 0.025f;
    float tile_vertices[] = {
        -tile_width / 2.0f, +tile_width / 2.0f, +tile_depth / 2.0f,
        +tile_width / 2.0f, +tile_width / 2.0f, +tile_depth / 2.0f,
        -tile_width / 2.0f, -tile_width / 2.0f, +tile_depth / 2.0f,
        +tile_width / 2.0f, -tile_width / 2.0f, +tile_depth / 2.0f,
        -tile_width / 2.0f, +tile_width / 2.0f, -tile_depth / 2.0f,
        +tile_width / 2.0f, +tile_width / 2.0f, -tile_depth / 2.0f,
        -tile_width / 2.0f, -tile_width / 2.0f, -tile_depth / 2.0f,
        +tile_width / 2.0f, -tile_width / 2.0f, -tile_depth / 2.0f,
    };
    unsigned int tile_colour_indicies[] = {
        0, 1, 2, // Front triangle
        4, 5, 6  // Rear triangle
    };
    unsigned int tile_blank_indicies[] = {
        1, 3, 2, // Front blank
        5, 7, 6, // Rear blank
        0, 1, 4,
        1, 3, 5,
        3, 2, 7,
        2, 0, 6,
        4, 5, 1,
        5, 7, 3,
        7, 6, 2,
        6, 4, 0 
    };
    
    unsigned int VAO_tile_blank, VAO_tile_colour;
    unsigned int EBO_tile_blank, EBO_tile_colour;
    unsigned int VBO_tile;
    glGenVertexArrays(1, &VAO_tile_blank);
    glGenVertexArrays(1, &VAO_tile_colour);
    glGenBuffers(1, &EBO_tile_blank);
    glGenBuffers(1, &EBO_tile_colour);
    glGenBuffers(1, &VBO_tile);


    glBindVertexArray(VAO_tile_blank);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_tile);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tile_vertices), tile_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_tile_blank);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tile_blank_indicies), tile_blank_indicies, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glBindVertexArray(VAO_tile_colour);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_tile);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tile_vertices), tile_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_tile_colour);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tile_colour_indicies), tile_colour_indicies, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // render loop
    // -----------
    float blue = 0.f;
    clock_t begin = clock();

    glm::mat4 trans = glm::mat4(1.0f);

    while (!glfwWindowShouldClose(window))
    {  
        blue = 0.5f +  (1.0f + sin( 0.00003f * double(clock()-begin) ))/4.0f;
        // input
        // -----
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        // ------
        glClearColor(0.2f, 0.2f, blue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int i, j;
        float dx, dy;
        float angle;
        for (i=0; i<16; i++) {
            dx = 0.125f*(i-7.5f);
            for (j=0; j<16; j++) {
                dy = 0.125*(j-7.5f);

                angle = 0.001f * double(clock()-begin);

                trans = glm::mat4(1.0f);
                trans = glm::translate(trans, glm::vec3(dx, dy, 0.0f));
                trans = glm::rotate(trans, glm::radians(angle*dx), glm::vec3(0.0, 1.0, 0.0));
                trans = glm::rotate(trans, glm::radians(angle*dy), glm::vec3(1.0, 0.0, 0.0));

                glUseProgram(shaderProgram);
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, glm::value_ptr(trans));
                glUniform3f(glGetUniformLocation(shaderProgram, "uColour"), 1.0f, 1.0f, 1.0f);
                glBindVertexArray(VAO_tile_blank);
                glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);

                glUniform3f(glGetUniformLocation(shaderProgram, "uColour"), blue, 0.0f, 0.0f);
                glBindVertexArray(VAO_tile_colour);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
 
                glBindVertexArray(0);
            }
        }

 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO_tile_blank);
    glDeleteVertexArrays(1, &VAO_tile_colour);
    glDeleteBuffers(1, &VBO_tile);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

