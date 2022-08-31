#ifndef SIMPLE_RENDER_SYSTEM_H
#define SIMPLE_RENDER_SYSTEM_H
 
#include "../Render/pipeline.h"
#include "../Render/device.h" 
#include "../GameAsset/gameObject.h"    //  -> contains model.h

#include <memory>
#include <vector>

namespace VULKVULK{
class SimpleRenderSystem{
    public: 
        SimpleRenderSystem(Device& device, VkRenderPass renderPass);
        ~SimpleRenderSystem();
        
        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;
        
        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject> &gameObjects);    //  we will get gameObjects from app using "loadGameObjects()"

    private: 
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

 
        Device &myDevice;  

        std::unique_ptr<Pipeline> myPipeline = nullptr;
        VkPipelineLayout myPipelineLayout; 
};

}   //  namespace VULKVULK


#endif