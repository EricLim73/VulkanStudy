#ifndef RENDERER_H
#define RENDERER_H

#include "../Render/window.h" 
#include "../Render/device.h"
#include "../Render/swapChain.h" 

#include <memory>
#include <vector>
#include <cassert>
    
namespace VULKVULK{
class Renderer{
    public: 
        Renderer(Window &window, Device &device);
        ~Renderer();
        //   BC we have vulkan Object within our class, we delete our copt constructor
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        //  split "drawFrame function"(which contains recordingCommandBuffer function) into 2 parts
        VkCommandBuffer beginFrame();
        void endFrame();
        //  Functions to call when recording swapChains renderPass
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        //  Getter Functions
        bool isFrameInProgress() const {return isFrameStarted;}
        int GetFrameIndex() const {
            assert(isFrameStarted && "Cannot get FrameBuffer Index when frame not in progress");
            return currentFrameIndex;
        }
        VkCommandBuffer GetCurrentBuffer() const {
            assert(isFrameStarted && "Cannot get command Buffer when frame not in progress");
            return myCommandBuffers[currentFrameIndex];
        }
        //  Render-SubSystem needs to access swapchain renderpass during pipeline Creation
        VkRenderPass GetSwapChainRenderPass() const {return mySwapChain->getRenderPass();}  
        float GetAspectRatio() const {return mySwapChain->extentAspectRatio();} //  to use windows W&H ratio for perspective matrix(fix stretching)

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain(); //swapchain, image, imageview, framebuffer, renderpass, depthsource, syncobject

        Window &myWindow;
        Device &myDevice;
        std::unique_ptr<SwapChain> mySwapChain = nullptr;  
        std::vector<VkCommandBuffer> myCommandBuffers; 

        uint32_t currentImageIndex{0};     //   to keep in check current frame in progress
        int currentFrameIndex{0};             //   index for FrameBuffer 0 ~ MAX_FRAME_IN_FLIGHT
        bool isFrameStarted{false};    
};

}   //  namespace VULKVULK

//  Basic Flow 
//  # During loop... 
//     1 -> beginFrame();
//     2 -> beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
//     3 -> endSwapChainRenderPass(VkCommandBuffer commandBuffer);
//     4 -> endFrame();


#endif