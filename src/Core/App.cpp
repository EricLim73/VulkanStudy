#include "App.h"
#include "core.h"

#include "../Render/simpleRenderSystem.h"
#include "../Render/camera.h"
#include "../IO/keyboard_movement.h"

#include <stdexcept>
#include <array>
#include <chrono>

namespace VULKVULK{

App::App(){
    loadGameObjects();
}

App::~App(){}

void App::run(){
    SimpleRenderSystem mySimpleRenderSystem(myDevice, myRenderer.GetSwapChainRenderPass());

    Camera cam{};
    //cam.setViewDirection(glm::vec3{0.0f}, glm::vec3{0.0f, 0.0f, 0.5f}); //  camera position in origin, facing positive Z but slightly right 
    //cam.setViewTarget(glm::vec3{-1.0f, -2.0f, -1.0f}, glm::vec3{0.0f, 0.0f, 0.5f});  //  camera position looking at target
    auto viewObject = GameObject::createGameObject();   //  object storing camera 

    KeyboardMovementController cameraController{};
    //  deltaTime
    auto currentTime = std::chrono::high_resolution_clock::now();


    //  Main Loop
    while(!myWindow.shouldClose()){
        glfwPollEvents();
        
        auto newTime = std::chrono::high_resolution_clock::now();   //  call after poll time to consider events 
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        //  View Transform
        cameraController.moveInPlaneXZ(myWindow.GetWindow(), frameTime, viewObject);
        cam.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);
        //  Projection Transform
        float aspect = myRenderer.GetAspectRatio();
        cam.setPerspectiveProjection(glm::radians(45.0f), aspect, 0.1f, 30.0f);

        //  if swapChain need recreation it returns nullptr
        if(auto commandBuffer = myRenderer.beginFrame()){
            myRenderer.beginSwapChainRenderPass(commandBuffer);
            mySimpleRenderSystem.renderGameObjects(commandBuffer, myGameObjects, cam);
            myRenderer.endSwapChainRenderPass(commandBuffer);
            myRenderer.endFrame();
        }
        
    }
    //  CPU blocks any operation(mainly deconstruction) so GPU operation can end before hand -> this will prevent error poppin up when closing window
    vkDeviceWaitIdle(myDevice.device());
}

std::unique_ptr<Model> createCubeModel(Device& device, glm::vec3 offset) {
    Model::bufferData bufferData{};
    //  Still use 4 for face -> total 24 vertex bc each face has different color
    //  if we use single color for all face == vertiex count goes down to 8
    bufferData.vertices = {
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},   

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},   

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    };
    for (auto& v : bufferData.vertices) {
      v.position += offset;
    }

    bufferData.indices = {  0,1,2,0,3,1,
                            4,5,6,4,7,5,
                            8,9,10,8,11,9,
                            12,13,14,12,15,13,
                            16,17,18,16,19,17,
                            20,21,22,20,23,21
                        };

    return std::make_unique<Model>(device, bufferData);
}



void App::loadGameObjects() {
    //std::shared_ptr<Model> model = createCubeModel(myDevice, {0.0f, 0.0f, 0.0f});
    std::shared_ptr<Model> model = Model::createModelFromFile(myDevice, "./src/GameAsset/Models/flat_vase.obj");//colored_cube
   
    auto gameObj = GameObject::createGameObject();
    gameObj.model = model;
    gameObj.transform.translation = {0.0f, 0.0f, 2.5f}; //  model transform(modelSpace -> worldSpace)
    gameObj.transform.scale = {0.5f, 0.5f, 0.5f};


    myGameObjects.push_back(std::move(gameObj));

}


}   //  namespace VULKVULK