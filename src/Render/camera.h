#ifndef CAMERA_H
#define CAMERA_H

#include "../Core/core.h"

namespace VULKVULK{
class Camera{
public:
    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far );
    void setPerspectiveProjection(float fov, float aspect, float near, float far);

    //  Camera facing direction 
    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
    //  Fixed Camera
    void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
    //  specify camera orientation
    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

    const glm::mat4& GetProjection() const {return projectionMatrix;}
    const glm::mat4& GetView() const {return viewMatrix;}

private:
    glm::mat4 projectionMatrix{1.0f};
    glm::mat4 viewMatrix{1.0f};


};

}


#endif