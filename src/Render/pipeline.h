#ifndef PIPELINE_H
#define PIPELINE_H

#include <string>
#include <vector>


#include "device.h"
#include "model.h"

namespace VULKVULK{
//  data used to configure pipeline => outside class for future use on application 
struct PipelineConfigInfo{
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
    VkPipelineViewportStateCreateInfo viewportInfo;
    //VkViewport viewport;  => when using "Dynamic Viewport", we will fill them with commandBuffers so no need for them to be here
    //VkRect2D scissor;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnable;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class Pipeline{
    public:
        Pipeline(
            Device& device, 
            const std::string& vertFilePath,
            const std::string& fragFilePath, 
            const PipelineConfigInfo& configInfo);
        ~Pipeline();
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;


        void bind(VkCommandBuffer commandBuffer);
        //  used outside to give defualt ConfigInfo
        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    private:
        //  For reading shader files
        static std::vector<char> readFile(const std::string& filePath);

        void createGraphicsPipeline(
            const std::string& vertFilePath,
            const std::string& fragFilePath, 
            const PipelineConfigInfo& configInfo);

        //  Initialize shaderModule with the given shaderCode
        void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
        
        Device& device;   //  Potentially memory unsafe(but we know that this will not get deleted before pipeline)
        VkPipeline graphicPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
};

}   //  namespace VULKVULK

#endif