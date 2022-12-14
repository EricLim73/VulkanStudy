#include "simpleRenderSystem.h"

#include <stdexcept>
#include <array>

namespace VULKVULK{

//  Push Constant -> PushConstant & Uniform(and some other data type) in vulkan need to follow alignment rules
struct SimplePushConstantData{
    glm::mat4 transform{1.f};
    glm::mat4 modelMatrix{1.f}; //  even though we need mat3, we're passing mat4 bc of alignment rulse

}; 


SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass renderPass) : myDevice(device){
    createPipelineLayout();
    createPipeline(renderPass);   
}

SimpleRenderSystem::~SimpleRenderSystem(){
    vkDestroyPipelineLayout(myDevice.device(), myPipelineLayout, nullptr);
    //  pipeline gets destoryed with its deconstructor->RAII
    //  Command buffer get destroyed with its CommandPool WHICH gets destoryed with Device
}


void SimpleRenderSystem::createPipelineLayout(){
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;   //  for if your using pushConstant range seperatly for shaders
    pushConstantRange.size = sizeof(SimplePushConstantData);
 
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;   //  pass data other than vertex data to our shaders(ex. texture, uniform buffer)
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;   //  efficiently push small data to our shader
    if(vkCreatePipelineLayout(myDevice.device(), &pipelineLayoutInfo, nullptr, &myPipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("Failed to create pipeline layout");
    }
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass){
    assert(myPipelineLayout != nullptr && "Can not create pipeline before pipelineLayout");

    //  using "swapChain Extent" width & height is important bc sometimes the windowScreen does not directly express screen Resolution(like apple monitors...)
    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.pipelineLayout = myPipelineLayout;
    pipelineConfig.renderPass = renderPass;
    myPipeline = std::make_unique<Pipeline>(
        myDevice, 
        "./shaders/compiledShaders/vert.spv",
        "./shaders/compiledShaders/frag.spv",
        pipelineConfig);
}

//  TODO:   Most camera calculations are done inside gpu, sending perspective & translation matrix to shader using uniform buffers
void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject> &gameObjects, const Camera& camera){
    
    myPipeline->bind(commandBuffer);
    //  VP transform
    auto projectionView = camera.GetProjection() * camera.GetView();

    //  loop through every gameObject
    for(auto& gameObject : gameObjects){
        SimplePushConstantData push{};
        auto modelMatrix = gameObject.transform.mat4();
        push.transform = projectionView * modelMatrix; //   MVP tranform
        push.modelMatrix = gameObject.transform.normalMatrix(); //  glm automatically converts mat3 to mat4

        vkCmdPushConstants(commandBuffer, myPipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        0, sizeof(SimplePushConstantData), &push);
        gameObject.model->bind(commandBuffer);
        gameObject.model->draw(commandBuffer);
    }
 
}

}   //  namespace VULKVULK