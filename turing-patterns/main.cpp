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
float initialConcArray[WORLD_HEIGHT][WORLD_WIDTH][4];

struct _concTextures {
    GLuint oldTextureID;
    GLuint newTextureID;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

GLuint loadComputeShader(std::string computeShaderPath);

struct _concTextures genConcTextures();
void initConcTexRandom(struct _concTextures concTextures);
void initConcTexSpots(struct _concTextures concTextures, int numSpots, float spotRadius);

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
    GLuint computeProgramID = loadComputeShader("turing.cs");
    if (computeProgramID == false)
        return -1;
    Shader ourShader("texture.vs", "texture.fs"); 
    struct _concTextures concTextures = genConcTextures();
    int spotsNum      = 100;
    float spotsRadius = 10.f;

    initConcTexSpots(concTextures, spotsNum, spotsRadius);

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
    float Da = 1.f;
    float Db = 100.f;
    float alpha = -0.005f;
    float beta = 10.f;
    int reactionMode = 0;
    int colourMode = 0;
    int chemicalView = 0;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui::Begin("Hello, world!");
        ImGui::Text("Is this working?");

        ImGui::InputInt("Reaction Mode", &reactionMode);
        ImGui::InputInt("Colour Mode",   &colourMode);
        ImGui::InputInt("Chemical View", &chemicalView);
        ImGui::InputFloat("dx",    &dx,    0.01f,    1.f,    "%.3f");
        ImGui::InputFloat("dt",    &dt,    0.00001f, 0.001f, "%.5f");
        ImGui::InputFloat("Da",    &Da,    0.01f,    1.f,    "%.3f");
        ImGui::InputFloat("Db",    &Db,    0.01f,    1.f,    "%.3f");
        ImGui::InputFloat("alpha", &alpha, 0.00001f, 0.001f, "%.5f");
        ImGui::InputFloat("beta",  &beta,  0.01f,    1.f,    "%.3f");

        ImGui::InputInt("Number of spots", &spotsNum);
        ImGui::InputFloat("Spot radius", &spotsRadius, 0.01f, 1.f, "%.3f");

        ImGui::End();

        for (int i = 0; i < 100; i++) {
            // Move 'new' data into 'old' texture
            glCopyImageSubData(concTextures.newTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,
                               concTextures.oldTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,
                               WORLD_WIDTH, WORLD_HEIGHT, 1);

            // Calculate 'new' data from 'old'
            glUseProgram(computeProgramID);

            glUniform1i(glGetUniformLocation(computeProgramID, "oldConc"), 0);
            glUniform1i(glGetUniformLocation(computeProgramID, "newConc"), 1);

            // Set simulation parameters - TODO: does this need to happen every time?
            glUniform1f(glGetUniformLocation(computeProgramID, "dx"),    dx);
            glUniform1f(glGetUniformLocation(computeProgramID, "dt"),    dt);
            glUniform1f(glGetUniformLocation(computeProgramID, "Da"),    Da);
            glUniform1f(glGetUniformLocation(computeProgramID, "Db"),    Db);
            glUniform1f(glGetUniformLocation(computeProgramID, "alpha"), alpha);
            glUniform1f(glGetUniformLocation(computeProgramID, "beta"),  beta);

            glUniform1i(glGetUniformLocation(computeProgramID, "reactionMode"), reactionMode);

            glBindImageTexture(0, concTextures.oldTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, concTextures.newTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

            glDispatchCompute((GLuint)WORLD_WIDTH / 32, (GLuint)WORLD_HEIGHT / 32, 1);

            // make sure writing to image has finished before read
            glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
            count++;
        }

        // render
        ourShader.use();
        ourShader.setInt("colourMode", colourMode);
        ourShader.setInt("chemicalView", chemicalView);
        glBindVertexArray(VAO); 
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, concTextures.newTextureID);

        std::cout << count << std::endl;

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (randomize_pending == true) {
            initConcTexSpots(concTextures, spotsNum, spotsRadius);
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

struct _concTextures genConcTextures()
{
    struct _concTextures conc;

    glGenTextures(1, &(conc.oldTextureID));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, conc.oldTextureID);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &(conc.newTextureID));
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, conc.newTextureID);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return conc;
}

void initConcTexRandom(struct _concTextures concTextures)
{
    // Populate 2D array of vec4s to store intial values of chemical concentations 
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            initialConcArray[y][x][0] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); // initial 'A' value
            initialConcArray[y][x][1] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); // initial 'B' value
            initialConcArray[y][x][2] = 0.0f;
            initialConcArray[y][x][3] = 0.0f;
        }
    }
    glBindTexture(GL_TEXTURE_2D, concTextures.newTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WORLD_WIDTH, WORLD_HEIGHT, 0, GL_RGBA, GL_FLOAT, initialConcArray);
    glBindTexture(GL_TEXTURE_2D, concTextures.oldTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WORLD_WIDTH, WORLD_HEIGHT, 0, GL_RGBA, GL_FLOAT, initialConcArray);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void initConcTexSpots(struct _concTextures concTextures, int numSpots, float spotRadius)
{
    if (numSpots < 0)
        numSpots = 0;

    // Populate 2D array of vec4s to store intial values of chemical concentations
    for (int y = 0; y < WORLD_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            initialConcArray[y][x][0] = 0.0f; // initial 'A' value
            initialConcArray[y][x][1] = 0.0f; // initial 'B' value
            initialConcArray[y][x][2] = 0.0f;
            initialConcArray[y][x][3] = 0.0f;
        }
    }

    // Set initial conc values inside of spots
    float x0, y0;
    float spotRadiusSquared = spotRadius * spotRadius;
    for (int i = 0; i < numSpots; i++) {
        x0 = WORLD_WIDTH  * static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        y0 = WORLD_HEIGHT * static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int x = 0; x < WORLD_WIDTH; x++) {
                if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= spotRadiusSquared) {
                    initialConcArray[y][x][0] = 0.5f;
                    initialConcArray[y][x][1] = 0.25f;
                }
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, concTextures.newTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WORLD_WIDTH, WORLD_HEIGHT, 0, GL_RGBA, GL_FLOAT, initialConcArray);
    glBindTexture(GL_TEXTURE_2D, concTextures.oldTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WORLD_WIDTH, WORLD_HEIGHT, 0, GL_RGBA, GL_FLOAT, initialConcArray);
    glBindTexture(GL_TEXTURE_2D, 0);
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