#include "camera.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

float Camera::getFov() const { return m_fov; }
void Camera::setFoV(const float f) { m_fov = f; }
float Camera::getAspectRatio() const { return m_aspectRatio; }
void Camera::setAspectRatio(const float a) { m_aspectRatio = a; }
float Camera::getNear() const { return m_near; }
void Camera::setNear(const float n) { m_near = n; }
float Camera::getFar() const { return m_far; }
void Camera::setFar(const float n) { m_far = n; }
void Camera::setPosition(const glm::vec3 &p) { m_pos = p; }
void Camera::setFront(const glm::vec3 &f) { cameraFront = f; }
glm::vec3 Camera::getFront() { return cameraFront; }
glm::vec3 Camera::getPosition() { return m_pos; }
void Camera::setUp(const glm::vec3 &u) { cameraUp = u; }
glm::vec3 Camera::getUp() { return cameraUp; }

glm::mat4 Camera::computeViewMatrix() const
{
    // return glm::lookAt(m_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    return glm::lookAt(m_pos, m_pos + cameraFront, cameraUp);
}

// Returns the projection matrix stemming from the camera intrinsic parameter.
glm::mat4 Camera::computeProjectionMatrix() const
{
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
}