#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

const unsigned int SCR_WIDTH  = 1024;
const unsigned int SCR_HEIGHT = 1024;

#define WORLD_WIDTH   1024
#define WORLD_HEIGHT  1024
float initialWorldArray[WORLD_HEIGHT][WORLD_WIDTH][4];

struct _worldTextures {
    GLuint oldTextureID;
    GLuint newTextureID;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

GLuint loadComputeShader(std::string computeShaderPath);
void zeroWorldArray();
void drawWorldArraySpot(float, float, float);
void addWorldBoundary();
void writeWorldArrayToTex(struct _worldTextures worldTextures);

struct _worldTextures genWorldTextures();
void initWorldTexRandom(struct _worldTextures worldTextures);

bool randomize_pending = false;

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwDefaultWindowHints();
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ahhhhh!", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // build and compile our shader programs and texure
    GLuint computeProgramID = loadComputeShader("waves.cs");
    if (computeProgramID == false)
        return -1;
    Shader ourShader("texture.vs", "texture.fs"); 
    struct _worldTextures worldTextures = genWorldTextures();

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

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    srand((unsigned)time(NULL));

    // render loop
    clock_t begin = clock();
    clock_t accumilator = clock();
    int time_step = 300;
    int count = 0;

    float dx = 1.f;
    float dt = 0.0005f;
    float c = 100; // ms^-1
    int colourMode = 0;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui::Begin("Hello, world!");
        ImGui::Text("Is this working?");

        ImGui::InputInt("Colour Mode",   &colourMode);
        ImGui::InputFloat("dx",    &dx,    0.01f,    1.f,    "%.3f");
        ImGui::InputFloat("dt",    &dt,    0.00001f, 0.001f, "%.5f");
        ImGui::InputFloat("c",     &c,     0.01f,    1.f,    "%.3f");

        ImGui::End();

        for (int i = 0; i < 100; i++) {
            // Move 'new' data into 'old' texture
            glCopyImageSubData(worldTextures.newTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,
                               worldTextures.oldTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,
                               WORLD_WIDTH, WORLD_HEIGHT, 1);

            // Calculate 'new' data from 'old'
            glUseProgram(computeProgramID);

            glUniform1i(glGetUniformLocation(computeProgramID, "oldWorld"), 0);
            glUniform1i(glGetUniformLocation(computeProgramID, "newWorld"), 1);

            // Set simulation parameters - TODO: does this need to happen every time?
            glUniform1f(glGetUniformLocation(computeProgramID, "dx"),    dx);
            glUniform1f(glGetUniformLocation(computeProgramID, "dt"),    dt);
            glUniform1f(glGetUniformLocation(computeProgramID, "c"),     c);

            glBindImageTexture(0, worldTextures.oldTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, worldTextures.newTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

            glDispatchCompute((GLuint)WORLD_WIDTH / 32, (GLuint)WORLD_HEIGHT / 32, 1);

            // make sure writing to image has finished before read
            glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
            count++;
        }

        // render
        ourShader.use();
        ourShader.setInt("colourMode", colourMode);
        glBindVertexArray(VAO); 
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, worldTextures.newTextureID);

        std::cout << count << std::endl;

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (randomize_pending == true) {
            zeroWorldArray();
            drawWorldArraySpot(400, 100, 100);
            addWorldBoundary();

            writeWorldArrayToTex(worldTextures);
            randomize_pending = false;
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
    } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        randomize_pending = true;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

struct _worldTextures genWorldTextures()
{
    struct _worldTextures world;

    glGenTextures(1, &(world.oldTextureID));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, world.oldTextureID);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &(world.newTextureID));
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, world.newTextureID);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return world;
}

void zeroWorldArray(void)
{
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            initialWorldArray[y][x][0] = 0.0f; // initial 'u' value
            initialWorldArray[y][x][1] = 0.0f; // initial 'du/dt' value
            initialWorldArray[y][x][2] = 0.0f; // initial 'F' value
            initialWorldArray[y][x][3] = 0.0f; // initial 'boundary' value
        }
    }
}

void addWorldBoundary(void)
{
    for (int y = 500; y < 520; y++) {
        for (int x = 0; x < (WORLD_WIDTH) / 3; x++) {
            initialWorldArray[y][x][0] = 0.0f; // initial 'u' value
            initialWorldArray[y][x][1] = 0.0f; // initial 'du/dt' value
            initialWorldArray[y][x][2] = 0.0f; // initial 'F' value
            initialWorldArray[y][x][3] = 1.0f; // initial 'boundary' value
        }
        for (int x = ((WORLD_WIDTH)/3 + 20); x < (2*(WORLD_WIDTH)/3 -20); x++) {
            initialWorldArray[y][x][0] = 0.0f; // initial 'u' value
            initialWorldArray[y][x][1] = 0.0f; // initial 'du/dt' value
            initialWorldArray[y][x][2] = 0.0f; // initial 'F' value
            initialWorldArray[y][x][3] = 1.0f; // initial 'boundary' value
        }
        for (int x = (2*WORLD_WIDTH) / 3; x < WORLD_WIDTH; x++) {
            initialWorldArray[y][x][0] = 0.0f; // initial 'u' value
            initialWorldArray[y][x][1] = 0.0f; // initial 'du/dt' value
            initialWorldArray[y][x][2] = 0.0f; // initial 'F' value
            initialWorldArray[y][x][3] = 1.0f; // initial 'boundary' value
        }
    }
}

void add2WorldBoundary(void)
{
    for (int y = 500; y < 550; y++) {
        for (int x = 0; x < (WORLD_WIDTH - 50) / 2; x++) {
            initialWorldArray[y][x][0] = 0.0f; // initial 'u' value
            initialWorldArray[y][x][1] = 0.0f; // initial 'du/dt' value
            initialWorldArray[y][x][2] = 0.0f; // initial 'F' value
            initialWorldArray[y][x][3] = 1.0f; // initial 'boundary' value

            initialWorldArray[y][WORLD_WIDTH-x][0] = 0.0f; // initial 'u' value
            initialWorldArray[y][WORLD_WIDTH-x][1] = 0.0f; // initial 'du/dt' value
            initialWorldArray[y][WORLD_WIDTH-x][2] = 0.0f; // initial 'F' value
            initialWorldArray[y][WORLD_WIDTH-x][3] = 1.0f; // initial 'boundary' value
        }
    }
}


void drawWorldArraySpot(float x0, float y0, float r)
{
    // Set initial conc values inside of spots
    float spotRadiusSquared = r * r;
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= spotRadiusSquared) {
                initialWorldArray[y][x][0] = 1.0f;
                initialWorldArray[y][x][1] = 0.0f;
                initialWorldArray[y][x][2] = 0.0f;
                initialWorldArray[y][x][3] = 0.0f;
            }
        }
    }
}

void writeWorldArrayToTex(struct _worldTextures worldTextures)
{
    glBindTexture(GL_TEXTURE_2D, worldTextures.newTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WORLD_WIDTH, WORLD_HEIGHT, 0, GL_RGBA, GL_FLOAT, initialWorldArray);
    glBindTexture(GL_TEXTURE_2D, worldTextures.oldTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WORLD_WIDTH, WORLD_HEIGHT, 0, GL_RGBA, GL_FLOAT, initialWorldArray);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void initWorldTexRandom(struct _worldTextures worldTextures)
{
    // Populate 2D array of vec4s to store intial values of chemical concentations 
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            initialWorldArray[y][x][0] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); // initial 'A' value
            initialWorldArray[y][x][1] = 0.0f;
            initialWorldArray[y][x][2] = 0.0f;
            initialWorldArray[y][x][3] = 0.0f;
        }
    }
}

GLuint loadComputeShader(std::string computeShaderPath)
{
    std::ifstream     computeShaderFile;
    std::stringstream computeShaderStream;
    std::string       computeShaderCode;
    const char*       computeShaderString;

    int success;
    char infoLog[512];

    GLuint programID = glCreateProgram();
    GLuint computeShaderID = glCreateShader(GL_COMPUTE_SHADER);

    // open and read file
    computeShaderFile.open(computeShaderPath);
    computeShaderStream << computeShaderFile.rdbuf();
    computeShaderFile.close();

    // convert stream into string
    computeShaderCode   = computeShaderStream.str();
    computeShaderString = computeShaderCode.c_str();

    // compile compute shader
    glShaderSource(computeShaderID, 1, &computeShaderString, NULL);
    glCompileShader(computeShaderID);
    glGetShaderiv(computeShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(computeShaderID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    };

    // link compute shader
    glAttachShader(programID, computeShaderID);
    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPUTE::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }

    glDeleteShader(computeShaderID);
    
    return programID;
}