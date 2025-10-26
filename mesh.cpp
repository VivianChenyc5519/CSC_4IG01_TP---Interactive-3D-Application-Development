#include "mesh.h"
#include <iostream>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"

extern GLuint g_program;
extern Camera g_camera;

// Class that defines the attributes of a mesh
std::vector<unsigned int> Mesh::getIndices() const
{
    return m_triangleIndices;
}

void Mesh::init()
{                                 // generate buffers
    glGenVertexArrays(1, &m_vao); // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
    glBindVertexArray(m_vao);

    //  Generate a GPU buffer to store the positions of the vertices
    size_t positionSize = sizeof(float) * m_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
    // std::cout<<"The position size is: "<<positionSize<<std::endl;
    glGenBuffers(1, &m_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
    glBufferData(GL_ARRAY_BUFFER, positionSize, m_vertexPositions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    // initialize normal buffer
    size_t normalSize = sizeof(float) * m_vertexNormals.size(); // Gather the size of the buffer from the CPU-side vector
    glGenBuffers(1, &m_normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
    glBufferData(GL_ARRAY_BUFFER, normalSize, m_vertexNormals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);

    // initialize tex buffer
    size_t texSize = sizeof(float) * m_vertexTexCoords.size(); // Gather the size of the buffer from the CPU-side vector
    // std::cout << "The tex coords vertex size is: " << texSize << std::endl;
    // for (size_t i = 0; i < 5; ++i)
    // {
    //     std::cout << m_vertexTexCoords[i] << " ";
    // }
    std::cout << std::endl;
    glGenBuffers(1, &m_texCoordVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordVbo);
    glBufferData(GL_ARRAY_BUFFER, texSize, m_vertexTexCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(2);

    // generate EBO
    size_t indexBufferSize = sizeof(unsigned int) * m_triangleIndices.size();
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, m_triangleIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
};
void Mesh::render(const glm::mat4 &model, const glm::vec3 &lColor,
                  const glm::vec3 &emission, GLuint texture, std::string planet)
{
    glUseProgram(g_program);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
    const glm::vec3 camPosition = g_camera.getPosition();
    glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);
    const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
    const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(model));     // pass the model matrix to the GPU program
    glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
    glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program
    glUniform3f(glGetUniformLocation(g_program, "lColor"), lColor[0], lColor[1], lColor[2]);
    glUniform3fv(glGetUniformLocation(g_program, "emission"), 1, glm::value_ptr(emission));
    glm::vec3 lightPos = glm::vec3(0.0f); // if Sun is at origin
    glUniform3fv(glGetUniformLocation(g_program, "lightPos"), 1, glm::value_ptr(lightPos));
    if (planet == "earth")
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(g_program, "material.albedoTex"), 0);
    }
    else if (planet == "moon")
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(g_program, "material.albedoTex"), 1);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    //std::cout << "Planet " << planet << " 's texture id is " << texture << std::endl;
    glBindVertexArray(m_vao); // activate the VAO storing geometry data
    glDrawElements(GL_TRIANGLES, m_triangleIndices.size(), GL_UNSIGNED_INT, (void *)0);
};

std::shared_ptr<Mesh> Mesh::genSphere(const size_t resolution)
{
    std::shared_ptr<Mesh> meshPtr = std::make_shared<Mesh>();
    int sector = resolution; // horizontal (longitude)
    int stack = resolution;  // vertical (latitude)
    int radius = 1;          // unit sphere;
    float s, t;              // texCoords
    float sectorStep = 2 * M_PI / sector;
    float stackStep = M_PI / stack;
    float stackAngle, sectorAngle;
    // Generate positions and normals
    for (int i = 0; i <= stack; ++i)
    {
        // std::cout << "i is " << i << std::endl;
        stackAngle = M_PI / 2 - i * stackStep; // starting from pi/2 to -pi/2
        float xy = radius * cosf(stackAngle);  // r * cos(u)
        float z = radius * sinf(stackAngle);   // r * sin(u)
        for (int j = 0; j <= sector; ++j)
        {
            sectorAngle = j * sectorStep; // starting from 0 to 2pi

            float x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
            float y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
            meshPtr->m_vertexPositions.push_back(x);
            meshPtr->m_vertexPositions.push_back(y);
            meshPtr->m_vertexPositions.push_back(z);
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            meshPtr->m_vertexNormals.push_back(normal.x);
            meshPtr->m_vertexNormals.push_back(normal.y);
            meshPtr->m_vertexNormals.push_back(normal.z);

            s = (float)j / sector;
            t = (float)i / stack;
            meshPtr->m_vertexTexCoords.push_back(s);
            meshPtr->m_vertexTexCoords.push_back(t);
        }
    }

    // generate CCW index list of sphere triangles
    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    int k1, k2;
    for (int i = 0; i < stack; ++i)
    {
        k1 = i * (sector + 1); // beginning of current stack
        k2 = k1 + sector + 1;  // beginning of next stack

        for (int j = 0; j < sector; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                meshPtr->m_triangleIndices.push_back(k1);
                meshPtr->m_triangleIndices.push_back(k2);
                meshPtr->m_triangleIndices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stack - 1))
            {
                meshPtr->m_triangleIndices.push_back(k1 + 1);
                meshPtr->m_triangleIndices.push_back(k2);
                meshPtr->m_triangleIndices.push_back(k2 + 1);
            }
        }
    }
    return meshPtr;
};
