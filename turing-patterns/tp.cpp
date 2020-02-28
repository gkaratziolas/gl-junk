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
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH  = 1000;
const unsigned int SCR_HEIGHT = 1000;

int render_mode = 0;

#define WORLD_WIDTH   100
#define WORLD_HEIGHT  100

float newA[WORLD_HEIGHT][WORLD_WIDTH];
float newB[WORLD_HEIGHT][WORLD_WIDTH];
float oldA[WORLD_HEIGHT][WORLD_WIDTH];
float oldB[WORLD_HEIGHT][WORLD_WIDTH];

float dx    = 1;
float dt    = 0.00005;
float Da    = 1;
float Db    = 100;
float alpha = -0.005;
float beta  = 10;

void update_reaction() {
    //std::swap(newA, oldA);
    //std::swap(newB, oldB);

    for (int y=0; y<WORLD_HEIGHT; y++) {
        for (int x=0; x<WORLD_WIDTH; x++) {
            double L;
            int x0 = x-1;
            int x1 = x;
            int x2 = x+1;
            int y0 = y-1;
            int y1 = y;
            int y2 = y+1;

            // reflect out of bounds
            if (x0 < 0)
                x0 = x1;
            if (x2 >= WORLD_WIDTH)
                x2 = x1;
            if (y0 < 0)
                y0 = y1;
            if (y2 >= WORLD_HEIGHT)
                y2 = y1;

            L =   (newA[y][x2] - 2*newA[y][x1] + newA[y][x0]) / (dx*dx)
                + (newA[y2][x] - 2*newA[y1][x] + newA[y0][x]) / (dx*dx);

            //newA[y][x] = newA[y][x] + dt * (Da*L);
            oldA[y][x] = newA[y][x] + dt * (Da*L + newA[y][x] - newA[y][x]*newA[y][x]*newA[y][x] - oldB[y][x] + alpha);

            L =   (newB[y][x2] - 2*newB[y][x1] + newB[y][x0]) / (dx*dx)
                + (newB[y2][x] - 2*newB[y1][x] + newB[y0][x]) / (dx*dx);

            oldB[y][x] = newB[y][x] + dt * (Db*L + beta*(oldA[y][x]-newB[y][x]));
        }
    }
    for (int y=0; y<WORLD_HEIGHT; y++) {
        for (int x=0; x<WORLD_WIDTH; x++) {
            double L;
            int x0 = x-1;
            int x1 = x;
            int x2 = x+1;
            int y0 = y-1;
            int y1 = y;
            int y2 = y+1;

            // reflect out of bounds
            if (x0 < 0)
                x0 = x1;
            if (x2 >= WORLD_WIDTH)
                x2 = x1;
            if (y0 < 0)
                y0 = y1;
            if (y2 >= WORLD_HEIGHT)
                y2 = y1;

            L =   (oldA[y][x2] - 2*oldA[y][x1] + oldA[y][x0]) / (dx*dx)
                + (oldA[y2][x] - 2*oldA[y1][x] + oldA[y0][x]) / (dx*dx);

            //newA[y][x] = oldA[y][x] + dt * (Da*L);
            newA[y][x] = oldA[y][x] + dt * (Da*L + oldA[y][x] - oldA[y][x]*oldA[y][x]*oldA[y][x] - oldB[y][x] + alpha);

            L =   (oldB[y][x2] - 2*oldB[y][x1] + oldB[y][x0]) / (dx*dx)
                + (oldB[y2][x] - 2*oldB[y1][x] + oldB[y0][x]) / (dx*dx);

            newB[y][x] = oldB[y][x] + dt * (Db*L + beta*(oldA[y][x]-oldB[y][x]));
        }
    }
    return;
}

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
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("texture.vs", "texture.fs"); 

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
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    srand( (unsigned)time(NULL) );

    for (int y=0; y<WORLD_HEIGHT; y++) {
        for (int x=0; x<WORLD_WIDTH; x++) {
                newA[y][x] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                newB[y][x] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        }
    }

    // load and create a texture 
    // -------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WORLD_WIDTH, WORLD_HEIGHT, 0, GL_RED, GL_FLOAT, newA);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    // render loop
    // -----------
    clock_t begin = clock();
    clock_t accumilator = clock();
    int time_step = 300;
    int count = 0;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        ourShader.use();
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        for (int i=0; i<200; i++) {
            update_reaction();
            count += 1;
        }

        if (render_mode == 0) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WORLD_WIDTH, WORLD_HEIGHT, 0, GL_RED, GL_FLOAT, newA);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, WORLD_WIDTH, WORLD_HEIGHT, 0, GL_RED, GL_FLOAT, newB);
        }
        std::cout << count << std::endl;

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
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
    } else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        render_mode = 0;
    } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        render_mode = 1;
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
