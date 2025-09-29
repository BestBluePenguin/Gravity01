#ifndef PHYSICS_H
#define PHYSICS_H
#pragma once

#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


/// @brief Golden Ratio
constexpr float phi = 1.61803398875f;

/// @brief Gravitational Constant
constexpr float gravConst = 6.6743e-11f;


struct Body
{
    glm::vec3 position;
    glm::vec3 velocity;
    float mass;

    GLuint VBO, VAO, EBO;
    glm::vec4 color;
    float radius;
    unsigned int quality; //Quality of generated sphere 

    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    /// @brief Generates a physics body
    /// @param radius Radius of the body in meters
    /// @param mass mass in kilograms
    /// @param initPos Initial position in meters
    /// @param color Color
    /// @param initVel Initial velocity 
    /// @param quality Quality of the object
    Body(float radius, float mass,
        glm::vec3 initPos = glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
        glm::vec3 initVel = glm::vec3(0.0f, 0.0f, 0.0f), 
        unsigned int quality = 0);

    /// @brief Generates an icosphere
    void icosphere();

    /// @brief Generates a cube sphere
    void cubesphere();

    /// @brief Creates VBO, VAO, EBO from mesh
    void uploadMesh();

    /// @brief Draws the body
    /// @param shaderPorgram GL Shader program
    void render(GLuint shaderPorgram);

    /// @brief Updates body position
    void update(float dt);
};


/// @brief Creates Vertex Array Object, Vertex Buffer Object, Element Buffer Object
/// @param VAO Vertex Array Object
/// @param VBO Vertex Buffer Object
/// @param EBO Element Buffer Object
/// @param vertices vertices of the object
/// @param vertexCount Number of vertices
/// @param indices Vertex indices
/// @param indexCount Number of vertex indices
void createVAOVBOEBO(GLuint& VAO, GLuint& VBO, GLuint& EBO, 
    const float* vertices, size_t vertexCount, const unsigned int* indices, size_t indexCount);

//Physics functions
/// @brief Applies Newtonian Physics
/// @param bodies Bodies to be simulated
/// @param dt Time 
void applyPhysics(std::vector<Body>& bodies, float dt);


#endif //PHYSICS_H