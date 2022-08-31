#include "App.h"
#include "core.h"

#include "../Render/simpleRenderSystem.h"

#include <stdexcept>
#include <array>

namespace VULKVULK{

App::App(){
    loadGameObjects();
}

App::~App(){}

void App::run(){
    SimpleRenderSystem mySimpleRenderSystem(myDevice, myRenderer.GetSwapChainRenderPass());
    
    //  Main Loop
    while(!myWindow.shouldClose()){
        glfwPollEvents();
        //  if swapChain need recreation it returns nullptr
        if(auto commandBuffer = myRenderer.beginFrame()){
            myRenderer.beginSwapChainRenderPass(commandBuffer);
            mySimpleRenderSystem.renderGameObjects(commandBuffer, myGameObjects);
            myRenderer.endSwapChainRenderPass(commandBuffer);
            myRenderer.endFrame();
        }
    }
    //  CPU blocks any operation(mainly deconstruction) so GPU operation can end before hand -> this will prevent error poppin up when closing window
    vkDeviceWaitIdle(myDevice.device());
}

void App::loadGameObjects() {
    std::vector<Model::Vertex> vertices{
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, 
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, 
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    auto model = std::make_shared<Model>(myDevice, vertices);

    std::vector<glm::vec3> colors{
      {1.f, .7f, .73f},
      {1.f, .87f, .73f},
      {1.f, 1.f, .73f},
      {.73f, 1.f, .8f},
      {.73, .88f, 1.f}  //
    };
    for (auto& color : colors) {
        color = glm::pow(color, glm::vec3{2.2f});
    }

    for(int i = 0; i < 40; ++i){
        auto triangle = GameObject::createGameObject();
        triangle.model = model;
        triangle.color = colors[i % colors.size()]; 
        triangle.transform2d.scale =  glm::vec2(.5f) + i * 0.025f;
        triangle.transform2d.rotation = i * 0.25f * glm::pi<float>();   //  180/4+i

        myGameObjects.push_back(std::move(triangle));   //  default move operator is used for this purpose inside "GameObject.h"
    }
}


}   //  namespace VULKVULK