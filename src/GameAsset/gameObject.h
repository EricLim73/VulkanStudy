#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "../Render/model.h"    //  -> core -> glm
#include <memory>

namespace VULKVULK{

struct Transform2DComponent{
    glm::vec2 translation{};
    glm::vec2 scale{1.0f, 1.0f};
    float rotation;

    glm::mat2 mat2() {
        //                                                                                                     [X]   [O]
        //  GLM reads in coloum not in row. => so it read "scale.x & 0.0f inside second{}" as 1st input     1| 123   14
        //  ex> [1st row] 1 2 3 [2nd row] 4 5 6 => is not 3X2 mat, its 2X3 mat                              2| 456   25
        //  so if you want to add z axis then you should add 1 value to each row, not adding new {}         3|       36
        glm::mat2 scaleMat{ {scale.x, 0.0f},
                            {0.0f, scale.y} };  //  i(1,0) is (scale.x, 0) & j(0,1) is (0, scale.y)
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);
        glm::mat2 rotationMat{{c, s}, {-s, c}};

        return rotationMat * scaleMat;
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
    Transform2DComponent transform2d;

private:
    GameObject(id_t objId) : id(objId) {} 

    id_t id;    //  Each GameObject has unique ID specifing them 

};


}


#endif