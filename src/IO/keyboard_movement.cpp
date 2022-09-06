#include "keyboard_movement.h"

#include <limits>

namespace VULKVULK{
void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject){
    glm::vec3 rotate{0};

    if(glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
    if(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
    if(glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
    if(glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

    //  dot to itself(size of vector) to check if its non-zero -> normalizing zero is gonna fuck up equation 
    //      + epsilon to avoid comparing float value directly with zero
    if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()){
        //  normalize to match speed for diagnal looking(normalize == direction)
        gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
    }

    gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());  // mod with 2pi => circle around 0~360

    float yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDir{sin(yaw), 0.0f, cos(yaw)};
    const glm::vec3 rigthDir{forwardDir.z, 0.0f, -forwardDir.x};    //  cross f & u
    const glm::vec3 upDir{0.0f, -1.0f, 0.0f};

    glm::vec3 moveDir{0.0f};
    
    if(glfwGetKey(window, keys.moveFoward) == GLFW_PRESS) moveDir += forwardDir;
    if(glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
    if(glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rigthDir;
    if(glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rigthDir;
    if(glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
    if(glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

    if(glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()){
        //  normalize to match speed for diagnal looking(normalize == direction)
        gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
    }
}

}