#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "../Render/model.h"    //  -> core -> glm
#include <memory>

namespace VULKVULK{
//  Model transform
struct TransformComponent{
    glm::vec3 translation{};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 rotation{};

    //  transform = scale * rotation * translation (calculate in <- direction )
    glm::mat4 mat4(){
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        
        //  Rotation uses Trait-Bryan Angle(Euler Angle)
        auto transform = glm::mat4{
            {   scale.x * (c1 * c3 + s1 * s2 * s3),
                scale.x * (c2 * s3),
                scale.x * (c1 * c2 * s3 - c3 * s1),
                0.0f
            },
            {
                scale.y * (c3 * s1 * s2 - c1 * s3),
                scale.y * (c2 * c3),
                scale.y * (c1 * c3 * s2 + s1 * s3),
                0.0f
            },
            {
                scale.z * (c2 * s1),
                scale.z * (-s2),
                scale.z * (c1 * c2),
                0.0f
            },
            {   //  last value == 1.0 move vector representing points
                translation.x, translation.y, translation.z, 1.0f   
            }
        };
        return transform;
    }
};

class GameObject{
public:
    using id_t = unsigned int;
    static GameObject createGameObject(){
        static id_t currentId = 0;
        return GameObject{currentId++}; //  starting from 0, every obj will have incrementing number of id
    }

    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = default; //  Use move operation for moving created gameObjects to vector storing all of them inside App
    GameObject& operator=(GameObject&&) = default;  
    
    id_t GetId() const {return id;}
 
    std::shared_ptr<Model> model{}; //  multiple game object can use same model -> model should be shared for convinience
    glm::vec3 color{};
    TransformComponent transform{};

private:
    GameObject(id_t objId) : id(objId) {} 

    id_t id;    //  Each GameObject has unique ID specifing them 
};


}


#endif