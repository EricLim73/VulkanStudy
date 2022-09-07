#ifndef MODEL_H
#define MODEL_H

#include "device.h"
#include "../core/core.h" 

#include <vector>

namespace VULKVULK{

//  Take vertex data from target -> allocate memory and copy given data to that memory
class Model{
public:
    struct Vertex{
        glm::vec3 position;
        glm::vec3 color;

        //  Return function for pipeline
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    struct bufferData{
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
    };
    Model(Device& _device, const Model::bufferData& bData);
    ~Model();
    
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    //  Basically does what VAO does in opengl
    void bind(VkCommandBuffer commandBuffer);
    //  same as Draw call in opengl
    void draw(VkCommandBuffer commandBuffer);

private:
    void createVertexBuffers(const std::vector<Vertex>& vertices);
    void createIndexBuffer(const std::vector<uint32_t>& indices);

    Device& device;
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;

    VkDeviceMemory vertexBufferMemory;  
    VkDeviceMemory indexBufferMemory;

    uint32_t vertexCount;
    uint32_t indexCount;
    bool hasIndexBuffer = false;

};

}
//  NOTE: that bufferMemory is seperate object and is not part of the buffer object when it gets created
//          -> This allows programmers control with memory allocation
#endif