#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


constexpr float phi = 1.61803398875f; //Golden Ratio
constexpr int tickRate = 30; //updates per second

//Screen Dim
constexpr unsigned int WIDTH = 800;
constexpr unsigned int HEIGHT = 600;

//Camera Properties
glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f,1.0f,0.0f);

float radius = 5.0f;
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

/// @brief Creates Vertex Array Object, Vertex Buffer Object, Element Buffer Object
/// @param VAO Vertex Array Object
/// @param VBO Vertex Buffer Object
/// @param EBO Element Buffer Object
/// @param vertices Vertecies of the object
/// @param vertexCount Number of vertecies
/// @param indices Vertex indices
/// @param indexCount Number of vertex indices
void createVAOVBOEBO(GLuint& VAO, GLuint& VBO, GLuint& EBO, 
    const float* vertices, size_t vertexCount, const unsigned int* indices, size_t indexCount)
{
    //VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);

    //EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(unsigned int), indices, GL_STATIC_DRAW);

    //Vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

}

//TODO Move to own file
//Physics object 

/// @brief Creates a physics body. Uses Icosphere or cube spheres.
class Body {
    public:
    GLuint VBO, VAO, EBO;
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
    unsigned int subdivision;
    glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    float radius;

    //TODO Change
    std::vector<glm::vec3> vertecies;
    std::vector<unsigned int> indices;

    /// @brief Constructor for object
    /// @param radius Radius of object
    /// @param subdivision Number of subdivisions
    Body(float radius, unsigned int subdivision)
    {
        this->radius = radius;
        this->subdivision = subdivision;
    }
    
    /// @brief Generates an Isosphere
    /// @param r radius
    /// @param subdivisons  number of subdivisions
    void icosphere(float r, unsigned int subdivisions = 2)
    {
        //icosahedron vertecies
        vertecies = {
            //ZY Plane
            glm::vec3(0.0f,  1.0f, phi),
            glm::vec3(0.0f, -1.0f, phi),
            glm::vec3(0.0f,  1.0f, -phi),
            glm::vec3(0.0f, -1.0f, -phi),
            //XY plnae
            glm::vec3( 1.0f,  phi, 0.0f),
            glm::vec3(-1.0f,  phi, 0.0f),
            glm::vec3( 1.0f, -phi, 0.0f),
            glm::vec3(-1.0f, -phi, 0.0f),
            //XZ plane
            glm::vec3( phi, 0.0f,  1.0f),
            glm::vec3( phi, 0.0f, -1.0f),
            glm::vec3(-phi, 0.0f,  1.0f),
            glm::vec3(-phi, 0.0f, -1.0f),
        };
        //Normalize     
        for (glm::vec3& cord : vertecies) cord = glm::normalize(cord);
        //TODO Subdividing

        //TODO Procedually generated Indices
        indices = {
            0, 11, 5,
            0, 5, 1,
            0, 1, 7,
            0, 7, 10,
            0, 10, 11,

            1, 5, 9,
            5, 11, 4,
            11, 10, 2,
            10, 7, 6,
            7, 1, 8,

            3, 9, 4,
            3, 4, 2,
            3, 2, 6,
            3, 6, 8,
            3, 8, 9,

            4, 9, 5,
            2, 4, 11,
            6, 2, 10,
            8, 6, 7,
            9, 8, 1
        };
        //TODO Scaling
    }

    void uploadMesh()
    {
        //Fattening vector
        std::vector<float> vertexData;
        for (const glm::vec3& v : vertecies)
        {
            vertexData.push_back(v.x);
            vertexData.push_back(v.y);
            vertexData.push_back(v.z);
        }
        createVAOVBOEBO(VAO, VBO, EBO, vertexData.data(), vertexData.size(), indices.data(), indices.size());
        
    }

    void render(GLuint shaderPorgram)
    {
        //Colorize sphere
        GLint colorLoc = glGetUniformLocation(shaderPorgram, "objectColor");
        glUniform4f(colorLoc, color.r, color.g, color.b, color.a);

        //Draws the mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    }

};

void updateCamera(GLuint shaderProgram) {
    // Calculate new camera position in spherical coordinates
    cameraPos.x = radius * sin(phiA) * cos(theta);
    cameraPos.y = radius * cos(phiA);
    cameraPos.z = radius * sin(phiA) * sin(theta);

    //Camera view matrix
    glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}



//TODO Cube Sphere


int main(int, char**){
    //Setup
    GLFWwindow* window = startGLFW();
    GLuint shaderProgram = createShaderProgram("vertex.glsl", "fragment.glsl");

    //Generation of a single sphere
    Body sphere1(1.0f, 0);
    sphere1.icosphere(1.0f);
    sphere1.uploadMesh();

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
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

        // Update the camera position based on the angles
        updateCamera(shaderProgram);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        sphere1.render(shaderProgram);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;

}
