#include "renderer.h"
#include <stdexcept>
#include <array>

 
namespace VULKVULK{

Renderer::Renderer(Window &window, Device &device) : myWindow(window), myDevice(device){
    recreateSwapChain();
    createCommandBuffers();
}

Renderer::~Renderer(){
    //  When App gets destroyed -> device->commandPool->CommandBuffer gets destroyed in order
    //  BUT now there is a chance where Renderer is gone but Application still running => so need to call freeCommand directly
   freeCommandBuffers();    
}

void Renderer::recreateSwapChain(){
    auto extent = myWindow.getExtent();
    //  wait until size one or more dimension sizable -> ex. wait when window is minimized
    while(extent.width == 0 || extent.height == 0){
        extent = myWindow.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(myDevice.device());    //  wait until current swapChain is no longer used
    if(mySwapChain == nullptr){ //  if its start of application, or there is no swapChain to reuse 
        mySwapChain = std::make_unique<SwapChain>(myDevice, extent);
    }
    else{
        std::shared_ptr<SwapChain> oldSwapChain = std::move(mySwapChain);
        mySwapChain = std::make_unique<SwapChain>(myDevice, extent, oldSwapChain);
        
        if(!oldSwapChain->compareSwapFormats(*mySwapChain.get())){   //  NOTE:   get() returns rawPointer of unique_ptr -> dereference with *
            throw std::runtime_error("Swap Chain image || depth format has changed and it is not compatible!");
            //  TODO: When creating check if renderPass that got recreated is compatible with current pipeline 
            //        Create Callback function to App alerting it is not compatible and recreate pipeline to match the changed Format
            //      -> if yes then no need to create new pipeline ( if renderPass are compatible, single pipeline can use either of them just fine )
            //  createPipeline(); // -> current use of render Pass is almost always compatible
        }

        //  No need to worry about if imageCount got altered -> we will now on look only the "MAX_FRAME_IN_FLIGHT" count -> which is const
    }
}

void Renderer::createCommandBuffers(){
    myCommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);  //  resize to match Max Frame in Flight

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //  Primar can submit to queue , secondary cant submit but can get called by other commandBuffers
    allocInfo.commandPool = myDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(myCommandBuffers.size());

    if(vkAllocateCommandBuffers(myDevice.device(), &allocInfo, myCommandBuffers.data()) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate command buffers");
    }

}

void Renderer::freeCommandBuffers(){
    vkFreeCommandBuffers(myDevice.device(), myDevice.getCommandPool(), static_cast<uint32_t>(myCommandBuffers.size()), myCommandBuffers.data());
    myCommandBuffers.clear();
}

VkCommandBuffer Renderer::beginFrame(){
    assert(!isFrameStarted && "Cant start Frame if its already in progress");

    //  Fetch next image(framebuffer) to draw
    auto result = mySwapChain->acquireNextImage(&currentImageIndex);
   
    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
        return nullptr; //  tells frame did not started successfully
    }
    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error("Failed to aquire image from swapChain");
    }

    isFrameStarted = true;
    auto commandBuffer = GetCurrentBuffer();    //  returns currentFrame's commandBuffer

    //  set recording   -> start of recording buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
        throw std::runtime_error("Failed to begin recording command buffer");
    }

    return commandBuffer;
}

void Renderer::endFrame(){
    assert(isFrameStarted && "Cant end Frame when no frame is in progress");
    auto commandBuffer  = GetCurrentBuffer();
    //  end recording -> end of recording buffer
    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to end command buffer recording");
    }

    auto result = mySwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || myWindow.wasFramebufferResized()){
        myWindow.resetFramebufferResized();
        recreateSwapChain();
    }
    else if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to submit command to queue");
    }
    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT; 
} 

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer){
    assert(isFrameStarted && "Cant start renderpass if no frame is in progress");
    assert(commandBuffer == GetCurrentBuffer() && "Cant begin renderPass on Command Buffer from different Frame");
    
    //  set which renderpass is getting used
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mySwapChain->getRenderPass();
    //  set which framebuffer our set renderpass is writting
    renderPassInfo.framebuffer = mySwapChain->getFrameBuffer(currentImageIndex);
    //  set area where shader "loads & stores" take place
    renderPassInfo.renderArea.extent = mySwapChain->getSwapChainExtent();
    
    //  set how we initialize our framebuffer attachment
    std::array<VkClearValue, 2> clearValues{};
    //  we only set color at "0" and depth at "1" bc thats the index we set inside renderpass(check swapChain.cpp)
    //  clearValues[0].color & clearValues[0].depthStencil are union type, so filling in depthStencil will corrupt color value
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 0.1f};    
    clearValues[1].depthStencil = {1.0f, 0}; // far,close 
    
    //  set inside renderPassInfo
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    //  Begin recording 
    //      -> this part is where renderPass instance is internally created insid vulkan
    // 3rd param => 2options for subpass, either commands are embedded inside primary or secondary => NO renderPass uses both CommandBuffer
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); 
    
    //  Configure Dynamic Viewport & Scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(mySwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(mySwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;    //  for depth range in viewport
    viewport.maxDepth = 1.0f;
    //  Configure Scissor   => cuts off outside boundary
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {mySwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer){
    assert(isFrameStarted && "Cant end renderpass when no frame is in progress");
    assert(commandBuffer == GetCurrentBuffer() && "Cant end renderPass on Command Buffer from different Frame");
    
    // End Recording -> first end renderPass
    vkCmdEndRenderPass(commandBuffer);
}




}   //  namespace VULKVULK