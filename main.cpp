// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (DO NOT distribute!)
//
// Copyright 2020-2025 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>
#include <array>
#include "mesh.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// constants
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 10;
const static float kRadOrbitMoon = 2;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

float earthRotation, earthOrbit, moonRotation, moonOrbit;
GLuint g_earthTexID, g_moonTexID;
bool firstMouse;
float lastX = 0, lastY = 0;
float yaw = -90.0f;
float pitch = 0.0f;

// Model transformation matrices
glm::mat4 g_sun, g_earth, g_moon;

// Window parameters
GLFWwindow *g_window = nullptr;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_ibo = 0;
GLuint g_colorVbo = 0;

// All vertex positions packed in one array [x0, y0, z0, x1, y1, z1, ...]
std::vector<float> g_vertexPositions;
// All triangle indices packed in one array [v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
std::vector<unsigned int> g_triangleIndices;

Camera g_camera;

std::shared_ptr<Mesh> earthptr = nullptr;
std::shared_ptr<Mesh> moonptr = nullptr;
std::shared_ptr<Mesh> sunptr = nullptr;

GLuint loadTextureFromFileToGPU(const std::string &filename)
{
  int width, height, numComponents;
  // Loading the image in CPU memory using stb_image
  unsigned char *data = stbi_load(
      filename.c_str(),
      &width, &height,
      &numComponents, // 1 for a 8 bit grey-scale image, 3 for 24bits RGB image, 4 for 32bits RGBA image
      0);

  GLuint texID;
  // TODO: create a texture and upload the image data in GPU memory using
  // glGenTextures, glBindTexture, glTexParameteri, and glTexImage2D
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if (data)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  }
  else
  {
    std::cout << "Failed to load texture" << std::endl;
  }

  // Free useless CPU memory
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

  return texID;
}

glm::vec3 computeDirection(float yaw, float pitch)
{
  glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  return direction;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow *window, int width, int height)
{
  // g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
  // glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
  int fbWidth, fbHeight;
  glfwGetFramebufferSize(g_window, &fbWidth, &fbHeight);
  g_camera.setAspectRatio((float)fbWidth / fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);
  lastX = fbWidth / 2;
  lastY = fbHeight / 2;
  //   GLint viewport[4];
  //   glGetIntegerv(GL_VIEWPORT, viewport);
  //   std::cout << "Viewport: x=" << viewport[0]
  //             << " y=" << viewport[1]
  //             << " width=" << viewport[2]
  //             << " height=" << viewport[3] << std::endl;
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS && key == GLFW_KEY_L)
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  else if (action == GLFW_PRESS && key == GLFW_KEY_F)
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  else if (action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q))
  {
    glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
  }
}

void errorCallback(int error, const char *desc)
{
  std::cout << "Error " << error << ": " << desc << std::endl;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
  if (firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }
  float xoffset = xpos - lastX;
  float yoffset = ypos - lastY;
  lastX = xpos;
  lastY = ypos;

  const float sensitivity = 0.8f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
  {
    pitch = 89.0f;
  }
  if (pitch < -89.0f)
  {
    pitch = -89.0f;
  }
  glm::vec3 front = computeDirection(yaw, pitch);
  // std::cout << "front = (" << front.x << ", " << front.y << ", " << front.z << ")\n";
  g_camera.setFront(front);
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  glm::vec3 cameraPos = g_camera.getPosition();
  glm::vec3 cameraFront = g_camera.getFront();
  glm::vec3 cameraUp = g_camera.getUp();
  float cameraSpeed = static_cast<float>(10 * deltaTime);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  g_camera.setPosition(cameraPos);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  float fov = g_camera.getFov();
  fov -= (float)yoffset;
  if (fov < 1.0f)
    fov = 1.0f;
  if (fov > 45.0f)
    fov = 45.0f;
  g_camera.setFoV(fov);
  // g_camera.computeProjectionMatrix();
}

void initGLFW()
{
  glfwSetErrorCallback(errorCallback);

  // Initialize GLFW, the library responsible for window management
  if (!glfwInit())
  {
    std::cerr << "ERROR: Failed to init GLFW" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Before creating the window, set some option flags
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  // Create the window
  g_window = glfwCreateWindow(
      1024, 768,
      "Interactive 3D Applications (OpenGL) - Simple Solar System",
      nullptr, nullptr);
  if (!g_window)
  {
    std::cerr << "ERROR: Failed to open window" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
  glfwMakeContextCurrent(g_window);
  glfwSetWindowSizeCallback(g_window, windowSizeCallback);
  glfwSetKeyCallback(g_window, keyCallback);
  // tell GLFW to capture our mouse
  // glfwSetCursorPosCallback(g_window, mouse_callback);
  glfwSetScrollCallback(g_window, scroll_callback);
  // glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  /// glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void initOpenGL()
{
  // Load extensions for modern OpenGL
  if (!gladLoadGL(glfwGetProcAddress))
  {
    std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glCullFace(GL_BACK);    // Specifies the faces to cull (here the ones pointing away from the camera)
  glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
  // glDisable(GL_CULL_FACE);              // Disables face culling (based on the orientation defined by the CW/CCW enumeration).
  glDepthFunc(GL_LESS);                 // Specify the depth test for the z-buffer
  glEnable(GL_DEPTH_TEST);              // Enable the z-buffer test in the rasterization
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f); // specify the background color, used any time the framebuffer is cleared
  int width, height;
  glfwGetFramebufferSize(g_window, &width, &height);
  // std::cout << "the width is " << width << " and the height is " << height << std::endl;
  glViewport(0, 0, width, height);
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string &filename)
{
  std::ifstream t(filename.c_str());
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename)
{
  GLuint shader = glCreateShader(type);                                    // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
  std::string shaderSourceString = file2String(shaderFilename);            // Loads the shader source from a file to a C++ string
  const GLchar *shaderSource = (const GLchar *)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
  glShaderSource(shader, 1, &shaderSource, NULL);                          // load the vertex shader code
  glCompileShader(shader);
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
  }
  glAttachShader(program, shader);
  glDeleteShader(shader);
}

void initGPUprogram()
{
  g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
  loadShader(g_program, GL_VERTEX_SHADER, "vertexShader.glsl");
  loadShader(g_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");
  g_earthTexID = loadTextureFromFileToGPU("../media/earth.jpg");
  g_moonTexID = loadTextureFromFileToGPU("../media/moon.jpg");
  glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons
  glUseProgram(g_program);
  // TODO: set shader variables, textures, etc.
}

std::vector<float> g_vertexColors;
// Define your mesh(es) in the CPU memory
void initCPUgeometry()
{
  // TODO: add vertices and indices for your mesh(es)
  g_vertexPositions = {
      0.f, 0.f, 0.f,
      1.f, 0.f, 0.f,
      0.f, 1.f, 0.f};
  g_triangleIndices = {0, 1, 2};
  g_vertexColors = {
      1.f, 0.f, 0.f,
      0.f, 1.f, 0.f,
      0.f, 0.f, 1.f};
}

void initGPUgeometry()
{
  // Create a single handle, vertex array object that contains attributes,
  // vertex buffer objects (e.g., vertex's position, normal, and color)
  glGenVertexArrays(1, &g_vao); // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
  glBindVertexArray(g_vao);

  // Generate a GPU buffer to store the positions of the vertices
  size_t vertexBufferSize = sizeof(float) * g_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
  glGenBuffers(1, &g_posVbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
  glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, g_vertexPositions.data(), GL_DYNAMIC_READ);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);

  // Generate a GPU buffer to store the colors of the vertices
  glGenBuffers(1, &g_colorVbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_colorVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * g_vertexColors.size(), g_vertexColors.data(), GL_DYNAMIC_READ);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(1);

  // Same for an index buffer object that stores the list of indices of the
  // triangles forming the mesh
  size_t indexBufferSize = sizeof(unsigned int) * g_triangleIndices.size();
  glGenBuffers(1, &g_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, g_triangleIndices.data(), GL_DYNAMIC_READ);

  glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
}

void initCamera()
{
  int width, height;
  glfwGetWindowSize(g_window, &width, &height);
  g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

  g_camera.setPosition(glm::vec3(0.0, 0.0, 25.0));
  g_camera.setNear(0.1);
  g_camera.setFar(80.1);
}

void init()
{
  initGLFW();
  initOpenGL();
  // initCPUgeometry();
  earthptr = Mesh::genSphere();
  // std::cout << "Earth vertex size: "<<earthptr->getIndices().size()<<std::endl;
  sunptr = Mesh::genSphere();
  // std::cout << "Sun vertex size: "<<sunptr->getIndices().size()<<std::endl;
  moonptr = Mesh::genSphere();
  // std::cout << "Moon vertex size: "<<moonptr->getIndices().size()<<std::endl;
  initGPUprogram();
  // initGPUgeometry();
  earthptr->init();
  moonptr->init();
  sunptr->init();
  initCamera();
}

void clear()
{
  glDeleteProgram(g_program);
  glfwDestroyWindow(g_window);
  glfwTerminate();
}

// The main rendering call
void render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

  const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
  const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();

  glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
  glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program

  glBindVertexArray(g_vao);                                                   // activate the VAO storing geometry data
  glDrawElements(GL_TRIANGLES, g_triangleIndices.size(), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec)
{
  // std::cout << currentTimeInSec << std::endl;
  // rotation speeds
  const float earthRotationSpeed = 1.0f;
  const float earthOrbitSpeed = earthRotationSpeed / 2.0f;
  const float moonOrbitSpeed = earthRotationSpeed * 2.0f;
  const float moonRotationSpeed = moonOrbitSpeed;

  earthRotation = currentTimeInSec * earthRotationSpeed;
  earthOrbit = currentTimeInSec * earthOrbitSpeed;
  moonOrbit = currentTimeInSec * moonOrbitSpeed;
  moonRotation = currentTimeInSec * earthOrbitSpeed;

  deltaTime = currentTimeInSec - lastFrame;
  lastFrame = currentTimeInSec;
}

int main(int argc, char **argv)
{
  init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
  while (!glfwWindowShouldClose(g_window))
  {
    // animate
    update(static_cast<float>(glfwGetTime()));
    processInput(g_window);

    // Tilt in earth
    float tilt = glm::radians(-23.5f);

    // All planets have their poll pointing towards me for now so rotate them
    // by 90 degrees first

    // WORLD (r, t) -> LOCAL (r, t) -> SCALE

    // SUN
    glm::mat4 sunModel = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeSun));
    sunModel = glm::rotate(sunModel, glm::radians(-90.0f), glm::vec3(1, 0, 0));

    // EARTH
    // ORBIT ROTATE, ORBIT TRANSLATE
    // glm::mat4 earthModel = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeSun));
    // earthModel = glm::rotate(earthModel, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    glm::mat4 earthModel = glm::mat4(1.0f);
    earthModel = glm::rotate(earthModel, earthOrbit, glm::vec3(0, 1, 0));
    earthModel = glm::translate(earthModel, glm::vec3(kRadOrbitEarth, 0.0f, 0.0f));
    // LOCAL ROTATE, TRANSLATE
    earthModel = glm::rotate(earthModel, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    earthModel = glm::rotate(earthModel, tilt, glm::vec3(0, 1, 0));
    // Z is upwards now
    earthModel = glm::rotate(earthModel, earthRotation, glm::vec3(0, 0, 1));
    earthModel = glm::scale(earthModel, glm::vec3(kSizeEarth));

    // MOON
    glm::mat4 moonModel = glm::mat4(1.0f);
    // WORLD ROTATE TRANSLATE
    moonModel = glm::rotate(moonModel, earthOrbit, glm::vec3(0, 1, 0));
    moonModel = glm::translate(moonModel, glm::vec3(kRadOrbitEarth, 0.0f, 0.0f));
    moonModel = glm::rotate(moonModel, moonOrbit, glm::vec3(0, 1, 0));
    moonModel = glm::translate(moonModel, glm::vec3(kRadOrbitMoon, 0.0f, 0.0f));
    // LOCAL ROTATE TRANSLATE
    moonModel = glm::rotate(moonModel, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    moonModel = glm::rotate(moonModel, moonRotation, glm::vec3(0, 0, 1));
    moonModel = glm::scale(moonModel, glm::vec3(kSizeMoon));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    earthptr->render(earthModel, glm::vec3(0.33, 0.5, 0.18), glm::vec3(0.0f), g_earthTexID, "earth"); // green
    moonptr->render(moonModel, glm::vec3(0.3, 0.3, 0.7), glm::vec3(0.0f), g_moonTexID, "moon");       // blue
    sunptr->render(sunModel, glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.9f, 0.5f), 10, "sun");    // yellow
    glfwSwapBuffers(g_window);
    glfwPollEvents();
  }
  clear();
  return EXIT_SUCCESS;
}
