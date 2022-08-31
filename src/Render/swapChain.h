#pragma once

#include "device.h"

// vulkan headers
#include <vulkan/vulkan.h>

#include <memory>

// std lib headers
#include <string>
#include <vector>

namespace VULKVULK {

class SwapChain {
public:
    //  at most 2 command Buffers can be submitted  -> one for rendering, one for aquiring image
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;    
    
    SwapChain(Device &deviceRef, VkExtent2D windowExtent);
    SwapChain(Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous); // For aquiring old swapChain for better resize behavior   
    
    SwapChain(const SwapChain &) = delete;
    SwapChain& operator=(const SwapChain &) = delete;   
    ~SwapChain();   

    //  Return Functions
    VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
    VkRenderPass getRenderPass() { return renderPass; }
    VkImageView getImageView(int index) { return swapChainImageViews[index]; }
    size_t imageCount() { return swapChainImages.size(); }
    VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() { return swapChainExtent; }
    uint32_t width() { return swapChainExtent.width; }
    uint32_t height() { return swapChainExtent.height; } 
    float extentAspectRatio() {
      return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
    }

    //  Helper Functions
    VkFormat findDepthFormat();   
    VkResult acquireNextImage(uint32_t *imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
    //  Check if renderPass is compatible after swapChain recreation -> if not we need to recreate pipeline(which uses renderPass)
    bool compareSwapFormats(const SwapChain& swapChain) const {
        return swapChain.swapChainImageFormat == swapChainImageFormat &&
                swapChain.swapChainDepthFormat == swapChainDepthFormat;
    }

private:
    void init();    //  wrapper for [SwapChain] creation depending on param with old swapChain
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper Functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat swapChainImageFormat;
    VkFormat swapChainDepthFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkRenderPass renderPass;

    std::vector<VkImage> depthImages;
    std::vector<VkDeviceMemory> depthImageMemorys;
    std::vector<VkImageView> depthImageViews;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    Device &device;
    VkExtent2D windowExtent;

    VkSwapchainKHR swapChain;
    std::shared_ptr<SwapChain> oldSwapChain = nullptr;  //  store old swapChain

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
};

}  // namespace lve
