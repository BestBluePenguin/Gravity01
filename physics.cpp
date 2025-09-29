#include "physics.h"

//Physics functions
Body::Body(float radius, float mass, glm::vec3 initPos, glm::vec4 color, glm::vec3 initVel, unsigned int quality)
{
    this->mass = mass;
    this->radius = radius;
    position = initPos;
    this->color = color;
    velocity = initVel;
    this->quality = quality;
}

void Body::icosphere()
    {
    vertices.clear();
    indices.clear();
    //PreCompute normalized
    const float a = 1.0f/sqrt(1.0f + phi*phi);
    const float c = a * phi;
    //icosahedron vertices
    vertices = {
        {c, a, 0.0f},
        {0.0f, c, -a},
        {0.0f, c, a},
        {a, 0.0f, -c},
        {a, 0.0f, c},
        {c, -a, 0.0f},
        {-a, 0.0f, -c},
        {-c, a, 0.0f},
        {-a, 0.0f, c},
        {0.0f, -c, -a},
        {0.0f, -c, a},
        {-c, -a, 0.0f}
    };
    //Normalization
    for (glm::vec3& v : vertices) v = glm::normalize(v);

    indices = {
        0,  1,  2,
        0,  3,  1,
        0,  2,  4,
        3,  0,  5,
        0,  4,  5,
        1,  3,  6,
        1,  7,  2,
        7,  1,  6,
        4,  2,  8,
        7,  8,  2,
        9,  3,  5,
        6,  3,  9,
        5,  4,  10,
        4,  8,  10,
        9,  5,  10,
        7,  6,  11,
        7, 11,  8,
        11,  6,  9,
        8, 11, 10,
        10, 11,  9
    };

    //Subidision
    for (int i = 0; i < quality; ++i) {
        std::unordered_map<uint64_t, int> midpointCache;
        std::vector<unsigned int> newIndices;

        auto getMidpoint = [&](int i1, int i2) -> int {
            uint64_t key = (uint64_t)std::min(i1, i2) << 32 | std::max(i1, i2);
            auto it = midpointCache.find(key);
            if (it != midpointCache.end()) return it->second;

            glm::vec3 mid = glm::normalize((vertices[i1] + vertices[i2]) * 0.5f);
            vertices.push_back(mid);
            int index = vertices.size() - 1;
            midpointCache[key] = index;
            return index;
        };

        for (size_t j = 0; j < indices.size(); j += 3) {
            unsigned int i0 = indices[j];
            unsigned int i1 = indices[j + 1];
            unsigned int i2 = indices[j + 2];

            unsigned int a = getMidpoint(i0, i1);
            unsigned int b = getMidpoint(i1, i2);
            unsigned int c = getMidpoint(i2, i0);

            newIndices.insert(newIndices.end(), {
                i0, a, c,
                i1, b, a,
                i2, c, b,
                a, b, c
            });
        }

        indices = std::move(newIndices);
    }
    //Scaling
    if (radius > 0.0f) for (glm::vec3& v : vertices) v *= radius;
}

void Body::cubesphere()
{
}

void Body::uploadMesh()
{
    //Fattening vector
    std::vector<float> vertexData;
    for (const glm::vec3& v : vertices)
    {
        glm::vec3 n = glm::normalize(v);
        vertexData.push_back(v.x);
        vertexData.push_back(v.y);
        vertexData.push_back(v.z);

        vertexData.push_back(n.x);  // normal x
        vertexData.push_back(n.y);  // normal y
        vertexData.push_back(n.z);  // normal z
    }
    createVAOVBOEBO(VAO, VBO, EBO, vertexData.data(), vertexData.size(), indices.data(), indices.size());
    
    }

void Body::render(GLuint shaderPorgram)
{
    //Update model matrix
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    GLint modelLoc = glGetUniformLocation(shaderPorgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //Colorize sphere
    GLint colorLoc = glGetUniformLocation(shaderPorgram, "objectColor");
    glUniform4f(colorLoc, color.r, color.g, color.b, color.a);

    //Draws the mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

void Body::update(float dt)
{
    position += velocity * dt;
}

//Misc functions
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    glBindVertexArray(0);
}

void applyPhysics(std::vector<Body>& bodies, float dt)
{
    for(size_t i = 0; i < bodies.size(); i++)
    {
        for (size_t j = i+1; j < bodies.size(); j++)
        {
            glm::vec3 direction = bodies[j].position - bodies[i].position;
            float distSQ = glm::dot(direction, direction);
            direction = glm::normalize(direction);
            float force = gravConst * ((bodies[i].mass * bodies[j].mass)/distSQ);

            glm::vec3 accelI = direction * (force/bodies[i].mass);
            glm::vec3 accelJ = -direction * (force/bodies[j].mass);

            bodies[i].velocity += accelI * dt;
            bodies[j].velocity += accelJ * dt;
        }
    }
}
