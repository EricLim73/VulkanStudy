#ifndef APP_H
#define APP_H

#include "../Render/window.h"
#include "../Render/pipeline.h"
#include "../Render/device.h"
#include "../Render/swapChain.h"
//  #include "../Render/model.h"
#include "../GameAsset/gameObject.h"    //  -> contains model.h

#include <memory>
#include <vector>

namespace VULKVULK{
class App{
    public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 960;

        App();
       ~App();
       //   BC we have vulkan Object within our class, we delete our copt constructor
        App(const App&) = delete;
        App& operator=(const App&) = delete;

        void run();
    private:
        //void loadModels();
        void loadGameObjects();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);
        void renderGameObjects(VkCommandBuffer commandBuffer);


        Window myWindow{WIDTH, HEIGHT, "Hello Vulkan"};
        Device myDevice{myWindow};
        //  easy for us to recreate swapChain when resize happen -> constant change = heap is more easy to contorl
        //      BUT with small performance cost
        std::unique_ptr<SwapChain> mySwapChain = nullptr; 
        std::unique_ptr<Pipeline> myPipeline = nullptr;
        VkPipelineLayout myPipelineLayout;    
        std::vector<VkCommandBuffer> myCommandBuffers;
        //std::unique_ptr<Model> myModel = nullptr;
        std::vector<GameObject> myGameObjects;


};

}   //  namespace VULKVULK


#endif