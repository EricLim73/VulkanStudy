#include "camera.h"
#include <cassert>
#include <limits>

/**
 *      Calculations are done assuming that perspective matrix is aligned with the "Z" axis
 *  This simplifies the matrix values for calculation
 * 
 */

namespace VULKVULK{

void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far){
    projectionMatrix = glm::mat4{1.0f};
    projectionMatrix[0][0] = 2.f / (right - left);
    projectionMatrix[1][1] = 2.f / (bottom - top);
    projectionMatrix[3][0] = -(right + left) / (right - left);
    projectionMatrix[3][1] = -(right + left) / (right - left);
    projectionMatrix[3][2] = -near / (far - near);
}
    
void Camera::setPerspectiveProjection(float fov, float aspect, float near, float far){
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
    const float tanHalfFov = tan(fov / 2.0f);
    projectionMatrix = glm::mat4{0.0f};
    projectionMatrix[0][0] = 1.0f / (aspect * tanHalfFov);
    projectionMatrix[1][1] = 1.0f / tanHalfFov;
    projectionMatrix[2][2] = far / (far - near);
    projectionMatrix[2][3] = 1.0f;
    projectionMatrix[3][2] = -(far * near) / (far - near);
}


}