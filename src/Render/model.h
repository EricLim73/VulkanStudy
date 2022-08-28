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
        glm::vec2 position;
        glm::vec3 color;

        //  Return function for pipeline
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    Model(Device& _device, const std::vector<Vertex>& vertices);
    ~Model();
    
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    //  Basically does what VAO does in opengl
    void bind(VkCommandBuffer commandBuffer);
    //  same as Draw call in opengl
    void draw(VkCommandBuffer commandBuffer);

private:
    void createVertexBuffers(const std::vector<Vertex>& vertices);

    Device& device;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;  
    uint32_t vertexCount;


};

}
//  NOTE: that bufferMemory is seperate object and is not part of the buffer object when it gets created
//          -> This allows programmers control with memory allocation
#endif