#ifndef KEYBOARD_MOVEMENT_H
#define KEYBOARD_MOVEMENT_H

#include "../GameAsset/gameObject.h"
#include "../Render/window.h"

namespace VULKVULK
{
class KeyboardMovementController{
public: 
    struct KeyMapping{
        int moveLeft = GLFW_KEY_A;
        int moveRight = GLFW_KEY_D;
        int moveFoward = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        int moveUp = GLFW_KEY_E;
        int moveDown = GLFW_KEY_Q;
        int lookLeft = GLFW_KEY_LEFT;
        int lookRight = GLFW_KEY_RIGHT;
        int lookUp = GLFW_KEY_UP;
        int lookDown = GLFW_KEY_DOWN;
    };
    
    void moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject);

    KeyMapping keys{};
    float moveSpeed{3.f};
    float lookSpeed{1.5f};


};


} 


#endif