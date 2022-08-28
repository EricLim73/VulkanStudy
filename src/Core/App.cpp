#include "App.h"
#include <stdexcept>
#include <array>

namespace VULKVULK{

App::App(){
    loadModels();
    createPipelineLayout();
    recreateSwapChain(); //  createPipeline(); is called from this function
    createCommandBuffers();
}
App::~App(){
    vkDestroyPipelineLayout(myDevice.device(), myPipelineLayout, nullptr);
    //  pipeline gets destoryed with its deconstructor->RAII
    //  Command buffer get destroyed with its CommandPool WHICH gets destoryed with Device
}

void App::run(){
    while(!myWindow.shouldClose()){
        glfwPollEvents();
        drawFrame();
    }
    //  CPU blocks any operation(mainly deconstruction) so GPU operation can end before hand -> this will prevent error poppin up when closing window
    vkDeviceWaitIdle(myDevice.device());
}


void App::createPipelineLayout(){
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;   //  pass data other than vertex data to our shaders(ex. texture, uniform buffer)
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;   //  efficiently push small data to our shader
    if(vkCreatePipelineLayout(myDevice.device(), &pipelineLayoutInfo, nullptr, &myPipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("Failed to create pipeline layout");
    }
}

void App::createPipeline(){
    assert(mySwapChain != nullptr && "Can not create pipeline before swapChain");
    assert(myPipelineLayout != nullptr && "Can not create pipeline before pipelineLayout");

    //  using "swapChain Extent" width & height is important bc sometimes the windowScreen does not directly express screen Resolution(like apple monitors...)
    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.pipelineLayout = myPipelineLayout;
    pipelineConfig.renderPass = mySwapChain->getRenderPass();
    myPipeline = std::make_unique<Pipeline>(
        myDevice, 
        "./shaders/compiledShaders/vert.spv",
        "./shaders/compiledShaders/frag.spv",
        pipelineConfig);
}

void App::recreateSwapChain(){
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
        mySwapChain = std::make_unique<SwapChain>(myDevice, extent, std::move(mySwapChain));
        if(mySwapChain->imageCount() != myCommandBuffers.size()){
            freeCommandBuffers();
            createCommandBuffers();
        }
    }
    if(myPipeline != nullptr){
        myPipeline.reset(nullptr);
    }
    //  TODO: When creating check if renderPass that got recreated is compatible with current pipeline 
    //      -> if yes then no need to create new pipeline ( if renderPass are compatible, single pipeline can use either of them just fine )
    createPipeline(); 
}

void App::createCommandBuffers(){
    myCommandBuffers.resize(mySwapChain->imageCount());  //  resize to match swapChainImageCount(2~3 most cases)

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //  Primar can submit to queue , secondary cant submit but can get called by other commandBuffers
    allocInfo.commandPool = myDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(myCommandBuffers.size());

    if(vkAllocateCommandBuffers(myDevice.device(), &allocInfo, myCommandBuffers.data()) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate command buffers");
    }
}

void App::freeCommandBuffers(){
    vkFreeCommandBuffers(myDevice.device(), myDevice.getCommandPool(), static_cast<uint32_t>(myCommandBuffers.size()), myCommandBuffers.data());
    myCommandBuffers.clear();
}

void sierpinski(
    std::vector<Model::Vertex> &vertices, int depth,
    glm::vec2 left, glm::vec2 right, glm::vec2 top) {
    if (depth <= 0) {
      vertices.push_back({top});
      vertices.push_back({right});
      vertices.push_back({left});
    }
    else {
        auto leftTop = 0.5f * (left + top);
        auto rightTop = 0.5f * (right + top);
        auto leftRight = 0.5f * (left + right);
        sierpinski(vertices, depth - 1, left, leftRight, leftTop);
        sierpinski(vertices, depth - 1, leftRight, right, rightTop);
        sierpinski(vertices, depth - 1, leftTop, rightTop, top);
    }
}

void App::loadModels() {
    //sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
    std::vector<Model::Vertex> vertices{
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, 
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, 
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    myModel = std::make_unique<Model>(myDevice, vertices);
}

//  Now we record our commandBuffer every frame
void App::recordCommandBuffer(int imageIndex){
            //  set recording
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if(vkBeginCommandBuffer(myCommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS){
            throw std::runtime_error("Failed to begin recording command buffer");
        }
        //  set which renderpass is getting used
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mySwapChain->getRenderPass();
        //  set which framebuffer our set renderpass is writting
        renderPassInfo.framebuffer = mySwapChain->getFrameBuffer(imageIndex);
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
        vkCmdBeginRenderPass(myCommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); 
        
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
        vkCmdSetViewport(myCommandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(myCommandBuffers[imageIndex], 0, 1, &scissor);


        //  Bind pipeline
        myPipeline->bind(myCommandBuffers[imageIndex]);

        //  Draw Command
        myModel->bind(myCommandBuffers[imageIndex]);
        myModel->draw(myCommandBuffers[imageIndex]);


        // End Recording -> first end renderPass
        vkCmdEndRenderPass(myCommandBuffers[imageIndex]);
        if(vkEndCommandBuffer(myCommandBuffers[imageIndex]) != VK_SUCCESS){
            throw std::runtime_error("Failed to end command buffer recording");
        }
}

void App::drawFrame(){
    uint32_t imageIndex = 0;
    //  Fetch next image(framebuffer) to draw
    auto result = mySwapChain->acquireNextImage(&imageIndex);
   
    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        recreateSwapChain();
        return;
    }
    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error("Failed to aquire image from swapChain");
    }

    recordCommandBuffer(imageIndex);

    result = mySwapChain->submitCommandBuffers(&myCommandBuffers[imageIndex], &imageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || myWindow.wasFramebufferResized()){
        myWindow.resetFramebufferResized();
        recreateSwapChain();
        return;
    }
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to submit command to queue");
    }
}

}   //  namespace VULKVULK