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

    glm::mat4 mat4();
    //  Calculate Transpose(Inverse(modelMatrix)) == R*S^-1
    glm::mat3 normalMatrix();
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