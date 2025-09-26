#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const int tickRate = 30; //updates per second
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

//Camera Properties
glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);

//Shaders

/// @brief Loads files
/// @param filePath Path to file
/// @return String derived form file
std::string loadFile(const char* filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) 
    {
        std::cerr << "File failed to load" << std::endl;
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
    GLuint fSource = compileShader(GL_VERTEX_SHADER, fShader);

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
        glGetShaderInfoLog(shaderProgram, 512, nullptr, infoLog);
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

    glewExperimental = GL_TRUE;
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST);

    glfwMakeContextCurrent(window);

    return window;
};

/// @brief Creates Vertex Array Obj and Vertex Buffer Obj
/// @param VAO Vertex Array Object
/// @param VBO Vertex Buffer Object
/// @param vertices List of verticies
/// @param vertexCount Vertex count
void createVAOVBO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount)
{
    //VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);

    //Vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

}


//Generate Sphere
void sphere()
{

}

int main(int, char**){
    //Setup
    GLFWwindow* window = startGLFW();
    GLuint shaderProgram = createShaderProgram("vertex_shader.glsl", "fragment_shader.glsl");

    //Triangle data
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // Bottom left
         0.5f, -0.5f, 0.0f, // Bottom right
         0.0f,  0.5f, 0.0f  // Top center
    };

    GLuint VAO, VBO;


    while(!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VBO);
    glDeleteBuffers(1, &VBO);
    glDeleteShader(shaderProgram);
    glfwTerminate;
    return 0;

}
