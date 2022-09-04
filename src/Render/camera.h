#ifndef CAMERA_H
#define CAMERA_H

#include "../Core/core.h"

namespace VULKVULK{
class Camera{
public:
    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far );
    void setPerspectiveProjection(float fov, float aspect, float near, float far);

    const glm::mat4& GetProjection() const {return projectionMatrix;}

private:
    glm::mat4 projectionMatrix{1.0f};


};

}


#endif