#ifndef MESH_H
#define MESH_H

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glad/gl.h>

// Forward declare camera & global program if needed
extern GLuint g_program;
extern class Camera g_camera;

// Class that defines the attributes of a mesh
class Mesh
{
public: 
  std::vector<unsigned int> getIndices() const;
  Mesh() = default;
  void init();
  void render(const glm::mat4 &model, const glm::vec3 &lColor, const glm::vec3 &emission, GLuint texture, std::string planet);
  static std::shared_ptr<Mesh> genSphere(const size_t resolution = 16);

private:
  std::vector<float> m_vertexPositions;
  std::vector<float> m_vertexNormals;
  std::vector<unsigned int> m_triangleIndices;
  std::vector<float> m_vertexTexCoords;
  GLuint m_vao = 0;
  GLuint m_posVbo = 0;
  GLuint m_normalVbo = 0;
  GLuint m_texCoordVbo = 0;
  GLuint m_ibo = 0;
};

#endif // MESH_H