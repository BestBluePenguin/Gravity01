#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>

#include "physics.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/// @brief Tick rate per second
constexpr int tickRate = 30;

//Screen Dim
constexpr unsigned int WIDTH = 800;
constexpr unsigned int HEIGHT = 600;

//Camera Properties
glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);

float radius = 30.0f;
float theta = 0.0f;
float phiA = glm::radians(45.0f);
float cameraSpeed = 0.001f;

//TODO Move shaders to dedicated file
//TODO Add camera rotation via mouse drag

/// @brief Loads files
/// @param filePath Path to file
/// @return String derived form file
std::string loadFile(const char* filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) 
    {
        std::cerr << "File failed to load " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/// @brief Compiles a shader
/// @param type Type of shader
/// @param source Shader source file
/// @return Compiled shader
GLuint compileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    const char* scr = source.c_str();
    
    glShaderSource(shader, 1, &scr, nullptr);
    glCompileShader(shader);

    //Checks shader status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
    }
    
    return shader;
}

/// @brief creates a shader program based on given files
/// @param vertexSource Vertex shader source
/// @param fragmentSource Fragment shader source
/// @return shader program
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    //Load files
    std::string vShader= loadFile(vertexSource);
    std::string fShader= loadFile(fragmentSource);

    GLuint vSource = compileShader(GL_VERTEX_SHADER, vShader);
    GLuint fSource = compileShader(GL_FRAGMENT_SHADER, fShader);

    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vSource);
    glAttachShader(shaderProgram, fSource);
    glLinkProgram(shaderProgram);

    //Shader Error checking
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader creation error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vSource);
    glDeleteShader(fSource);

    return shaderProgram;
}

/// @brief Creation of a GLFW Window
/// @return GLFW Window
GLFWwindow* startGLFW()
{
    if(!glfwInit())
    {
        std::cerr<<"Failed to start window" << std::endl;
        return nullptr;
    }
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Gravity 3D", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    return window;
};

/// @brief Updates the camera
/// @param shaderProgram Shader proram
void updateCamera(GLuint shaderProgram) {
    // Calculate new camera position in spherical coordinates
    cameraPos.x = radius * sin(phiA) * cos(theta);
    cameraPos.y = radius * cos(phiA);
    cameraPos.z = radius * sin(phiA) * sin(theta);

    //Camera view matrix
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));
}

/// @brief Main function
int main(int, char**){
    //Setup
    GLFWwindow* window = startGLFW();
    GLuint shaderProgram = createShaderProgram("vertex.glsl", "fragment.glsl");
    glUseProgram(shaderProgram);

    //DEBUG
    /*
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    */

    //Generation of sphere
    std::vector<Body> bodies
    {
        {1.0f,1.0e13f,{0.0f,3.0f,0.0f},{0.30f,0.67f,0.89f,1.0f},{0.1f,0.0f,0.0f},2, true}, 
        {1.0f,5.0e2f,{0.0f,-9.0f,3.0f},{0.14f,0.679f,0.44f,1.0f},{5.0f,0.0f,0.0f},2, true}, 
        {0.5f,1.0e5f,{5.0f,-3.0f,0.0f},{0.71f,0.73f,0.75f,1.0f},{-10.0f,0.0f,0.0f},2}, 
        {0.75f,1.0e10f,{-10.0f,0.0f,0.0f},{0.30f,0.67f,0.89f,1.0f},{-2.56f,-0.78f,-4.69f},2}
    };

    for (Body& obj: bodies) 
    {
        obj.icosphere();
        obj.uploadMesh();
    }

    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    //Projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    //Decoupling physics
    float accum = 0.0f;
    std::chrono::high_resolution_clock::time_point prev = std::chrono::high_resolution_clock::now();
    //Render loop
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = now - prev;
        prev = now;

        accum += elapsed.count();

        //Camera rotation
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            std::cout << "LEFT arrow key pressed!" << std::endl;
            theta -= cameraSpeed;  // Rotate left
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            std::cout << "RIGHT arrow key pressed!" << std::endl;
            theta += cameraSpeed;  // Rotate right
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            std::cout << "UP arrow key pressed!" << std::endl;
            phiA += cameraSpeed;  // Rotate up (decrease angle from the Z-axis)
            if (phiA > glm::radians(89.0f)) {  // Limit the rotation to avoid flipping
                phiA = glm::radians(89.0f);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            std::cout << "DOWN arrow key pressed!" << std::endl;
            phiA -= cameraSpeed;  // Rotate down (increase angle from the Z-axis)
            if (phiA < glm::radians(1.0f)) {  // Limit the rotation to avoid flipping
                phiA = glm::radians(1.0f);
            }
        }

        //Decoupled physics
        while (accum >= (1.0f/tickRate))
        {
            applyPhysics(bodies,1.0f/tickRate);
            for (Body& obj: bodies) obj.update(1.0f/tickRate);
            accum -= (1.0f/tickRate);
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        updateCamera(shaderProgram);

        applyLight(shaderProgram, bodies);
        // Update the camera position based on the angles

        for (Body& obj: bodies) obj.render(shaderProgram);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;

}
