#include "model.h"

#include <cassert>
#include <cstring>

namespace VULKVULK{

Model::Model(Device& _device, const std::vector<Vertex>& vertices) : device(_device){
    createVertexBuffers(vertices);
}

Model::~Model(){
    vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
    vkFreeMemory(device.device(), vertexBufferMemory, nullptr);
}
    
void Model::createVertexBuffers(const std::vector<Vertex>& vertices){
    vertexCount = static_cast<uint32_t>(vertices.size());
    //  assert to check vertexCount is at least 3 (to form basic shape)
    assert(vertexCount >= 3 && "Vertex count must be at least 3!");

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;  //  bufferSize = sizeof 1 vertex * totalnumber
    //  HOST = CPU & DEVICE = GPU => HOST_VISIBLE will set vertex data accessible through cpu
    //                            => HOST_COHERENT wil keeps HOST&DEVIEC region consistent -> without this flag u need another way to keep coherency
    device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,  
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer, vertexBufferMemory );

    void* data;
    //  link HOST cpu "data" memory address to DEVICE gpu "vertexBufferMemoryz"
    vkMapMemory(device.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
    //  copy vertices data to "data" -> bc of "COHERENT" bit, automaitcally GPU memory has this data
    //       -> transfer does not happen right now, it happens at backGround but is guaranteed to be completed after next "VkQueueSubmit()" 
    memcpy(data, vertices.data(), static_cast<uint32_t>(bufferSize)); 
    vkUnmapMemory(device.device(), vertexBufferMemory); //  unlink data with gpu data
}

void Model::draw(VkCommandBuffer commandBuffer){
   vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0); 
}

void Model::bind(VkCommandBuffer commandBuffer){
    VkBuffer buffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};   //  starting offset to where binding starts
    //  record to commandBuffer to bind 
    //  [i]th value of "buffer"&"offset" to binding-index["first_binding" + i]
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

//  INFO:   [Pipeline] Need to provide pipeline with these descriptions -> inside "VkPipelineVertexInputStateCreateInfo"
//  This binding description corresponds to single VertexBuffer
std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions(){
    std::vector<VkVertexInputBindingDescription> bindingDescription(1);
    bindingDescription[0].binding = 0;  //  for binding index "0"
    bindingDescription[0].stride = sizeof(Vertex);  //  read size of Vertex as 1 item
    bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
//  you can also return 
//  {{0,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, position)},
//  {1,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, position)}}
//  the order is ["location", "binding", "format", "offset"]
}
std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions(){
    std::vector<VkVertexInputAttributeDescription> attributeDescription(2);
    attributeDescription[0].binding = 0;    
    attributeDescription[0].location = 0;   //  location inside vertex shader(position)
    attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;   //  indicating 2 float variable
    attributeDescription[0].offset = offsetof(Vertex, position);

    attributeDescription[1].binding = 0;    
    attributeDescription[1].location = 1;   //  location inside vertex shader(color)
    attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;   //  indicating 2 float variable
    attributeDescription[1].offset = offsetof(Vertex, color);
    return attributeDescription;
}




}