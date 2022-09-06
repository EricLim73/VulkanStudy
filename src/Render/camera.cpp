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
    auto aspect_nominator = aspect;
    auto aspect_denominator = 1.0f;
    if (aspect < 1.0f) {
        aspect_nominator = 1.0f;
        aspect_denominator = 1 / aspect;
    }
    const float tanHalfFovy = tan(fov / 2.f);
    projectionMatrix = glm::mat4{0.0f};
    projectionMatrix[0][0] = 1.f / (aspect_nominator * tanHalfFovy);
    projectionMatrix[1][1] = 1.f / (aspect_denominator * tanHalfFovy);
    projectionMatrix[2][2] = far / (far - near);
    projectionMatrix[2][3] = 1.f;
    projectionMatrix[3][2] = -(far * near) / (far - near);
} 

void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
    //  Setting Orthonormal Basis -> use this as base for rotation + multiply with position matrix to form the whole transform
    //  3 unit vector can describe 1 coordinate space
    const glm::vec3 w{glm::normalize(direction)};   //  camera's view direction unit            [z]
    const glm::vec3 u{glm::normalize(glm::cross(w, up))};   //  camera's right direction unit   [x]
    const glm::vec3 v{glm::cross(w, u)};    //  camera's up direction unit                      [y]
    viewMatrix = glm::mat4{1.f};
    viewMatrix[0][0] = u.x;
    viewMatrix[1][0] = u.y;
    viewMatrix[2][0] = u.z;
    viewMatrix[0][1] = v.x;
    viewMatrix[1][1] = v.y;
    viewMatrix[2][1] = v.z;
    viewMatrix[0][2] = w.x;
    viewMatrix[1][2] = w.y;
    viewMatrix[2][2] = w.z;
    viewMatrix[3][0] = -glm::dot(u, position);
    viewMatrix[3][1] = -glm::dot(v, position);
    viewMatrix[3][2] = -glm::dot(w, position);
}

void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    auto direction = target - position;
    if(glm::dot(direction, direction) > std::numeric_limits<float>::epsilon()){
        setViewDirection(position, target - position, up);
        return;
    }
    setViewDirection(position, target, up);
}

//  camera movement & rotation -> Trait-Bryan Euler Angle YXZ ( rotation vector in this context has all uvw )
void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
    const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
    const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
    viewMatrix = glm::mat4{1.f};
    viewMatrix[0][0] = u.x;
    viewMatrix[1][0] = u.y;
    viewMatrix[2][0] = u.z;
    viewMatrix[0][1] = v.x;
    viewMatrix[1][1] = v.y;
    viewMatrix[2][1] = v.z;
    viewMatrix[0][2] = w.x;
    viewMatrix[1][2] = w.y;
    viewMatrix[2][2] = w.z;
    viewMatrix[3][0] = -glm::dot(u, position);
    viewMatrix[3][1] = -glm::dot(v, position);
    viewMatrix[3][2] = -glm::dot(w, position);
    /*  -> rotation matrix is inversed so result is the world rotating according to the camera space origin 
        ->  "-" position bc now we are looking in camera space, world should move opposite amount from origin to match
        ux uy uz 0      1 0 0 px
        vx vy vz 0  *   0 1 0 py
        wx wy wz 0      0 0 1 pz
        0  0  0  1      0 0 0  1
    */
}

}