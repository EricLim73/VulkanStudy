#include "pipeline.h"

#include <fstream>
#include <stdexcept>
#include <cassert> 

namespace VULKVULK{

Pipeline::Pipeline(
            Device& device, 
            const std::string& vertFilePath,
            const std::string& fragFilePath,
            const PipelineConfigInfo& configInfo) : device(device){
    createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
}

Pipeline::~Pipeline(){
    vkDestroyShaderModule(device.device(), vertShaderModule, nullptr);
    vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);
    vkDestroyPipeline(device.device(), graphicPipeline, nullptr);
}

std::vector<char> Pipeline::readFile(const std::string& filePath){
    std::ifstream file{filePath.c_str(), std::ios::ate | std::ios::binary};

    if(!file.is_open()){
        throw std::runtime_error("failed to open file : " + filePath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());    //  std::ios::ate points to the EOF, tellg returns it giving us the total size
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}   

void Pipeline::createGraphicsPipeline(
    const std::string& vertFilePath, 
    const std::string& fragFilePath, 
    const PipelineConfigInfo& configInfo){
    
    auto vertCode = readFile(vertFilePath);
    auto fragCode = readFile(fragFilePath);

    assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
            "Cannot create graphics pipeline:: no pipeline Layout provided in configInfo");
    assert(configInfo.renderPass != VK_NULL_HANDLE &&
            "Cannot create graphics pipeline:: no renderPass provided in configInfo");

    CreateShaderModule(vertCode, &vertShaderModule);
    CreateShaderModule(fragCode, &fragShaderModule);

    //  Setting shader module
    VkPipelineShaderStageCreateInfo shaderStages[2];
    //  vertex shader
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShaderModule;
    shaderStages[0].pName = "main";  //  entry function in vertex shader
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;
    //  fragment shader
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShaderModule;
    shaderStages[1].pName = "main";  //  entry function in vertex shader
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;
    
    //  Specifying format of vertex Data -> sort of like VBO/VAO in opengl
    auto bindingDescriptions = Model::Vertex::getBindingDescriptions();
    auto attributeDescriptions = Model::Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()); 
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2; //  vert + frag
    pipelineInfo.pStages = shaderStages;    //  shaderStages is array so name == address
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
    pipelineInfo.pViewportState = &configInfo.viewportInfo;
    pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
    pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
    pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
    pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
    pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

    pipelineInfo.layout = configInfo.pipelineLayout;
    pipelineInfo.renderPass = configInfo.renderPass;
    pipelineInfo.subpass = configInfo.subpass;

    //  Optimization purpose => TL:DR less expensive  when making new pipeline from existing one
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    //  VK_NULL_HANDLE can be replaced for optimization(pipeline cache)
    if(vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicPipeline) != VK_SUCCESS){
        throw std::runtime_error("Failed to create graphics pipeline");
    } 
}

void Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule){
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    //  Vulkan expects uint32 type data. Even though data is char and uint32 is different,
    //  but vector<> default allocator ensures worst case allignment requirment c style string cant reinterpret_cast
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());    
    if(vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS){
        throw std::runtime_error("Failed to create shader module");
    }
}

void Pipeline::bind(VkCommandBuffer commandBuffer){
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline);
}


void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo){
    //  drawing(or operation) setting for given vertex data -> like GL_TRIANGLE inside drawCall from openGL
    configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;    //  everry 3point => triangle, if its 'strip', we get similar effect as using indicies
    configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE; //  TRUE will allow us to use specifiv indiceis for "strip" and "fan" behavior

    //  Configure viewport & scissor    => if filling in dynamically through commandBuffer, you dont need them here
    //configInfo.viewport.x = 0.0f;
    //configInfo.viewport.y = 0.0f;
    //configInfo.viewport.width = static_cast<float>(width);
    //configInfo.viewport.height = static_cast<float>(height);
    //configInfo.viewport.minDepth = 0.0f;    //  for depth range in viewport
    //configInfo.viewport.maxDepth = 1.0f;
    ////  Configure Scissor   => cuts off outside boundary
    //configInfo.scissor.offset = {0, 0};
    //configInfo.scissor.extent = {width, height};

    configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    configInfo.viewportInfo.viewportCount = 1;
    configInfo.viewportInfo.pViewports = nullptr;   // &configInfo.viewport;
    configInfo.viewportInfo.scissorCount = 1;
    configInfo.viewportInfo.pScissors = nullptr;    // &configInfo.scissor;

    //  Configure Rasterization
    configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;   //  clamp z axis 0~1 => need to enable specific gpu feature
    configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE; // Use when u only need the first few stage of pipeline
    configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;    //  draw option
    configInfo.rasterizationInfo.lineWidth = 1.0f;
    configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;  
    configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
    configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;    // Optional
    configInfo.rasterizationInfo.depthBiasClamp = 0.0f;             // Optional
    configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;       // Optional

    //  Configure Multisample
    configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.multisampleInfo.minSampleShading = 1.0f;
    configInfo.multisampleInfo.pSampleMask = nullptr;
    configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

    //  Configure ColorAttachment
    configInfo.colorBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
    configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    //  Configure ColorBlending
    configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
    configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    configInfo.colorBlendInfo.attachmentCount = 1;
    configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
    configInfo.colorBlendInfo.blendConstants[0] = 0.0f;     // Optional     
    configInfo.colorBlendInfo.blendConstants[1] = 0.0f;     // Optional 
    configInfo.colorBlendInfo.blendConstants[2] = 0.0f;     // Optional 
    configInfo.colorBlendInfo.blendConstants[3] = 0.0f;     // Optional 

    //  Configure DepthStencil
    configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.minDepthBounds = 0.0f;  //  Optional
    configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  //  Optional
    configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.front = {};     //  Optional
    configInfo.depthStencilInfo.back = {};      //  Optional

    //  Set DynamicStateInfo & dynamicStateEnable
    configInfo.dynamicStateEnable = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnable.data();
    configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnable.size());
    configInfo.dynamicStateInfo.flags = 0;

}


}   //  namespace VULKVULK