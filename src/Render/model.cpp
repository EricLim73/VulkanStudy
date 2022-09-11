#include "model.h"
#include "../Core/utils.h"


#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
//  to make a hashing function for our Vertex struct, we need to enable this -> to hash individual vec component
//  though experimental, it is considered safe
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>    //  chech for duplicate vertex data => if same dont save in vertex but save index id to indices

template <>
struct std::hash<VULKVULK::Model::Vertex> {
  size_t operator()(VULKVULK::Model::Vertex const &vertex) const {
    size_t seed = 0;
    VULKVULK::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
    return seed;
  }
};

namespace VULKVULK{

Model::Model(Device& _device, const Model::bufferData& bData) : device(_device){
    createVertexBuffers(bData.vertices);
    createIndexBuffer(bData.indices);
}

Model::~Model(){
    vkDestroyBuffer(device.device(), vertexBuffer, nullptr);   
    vkFreeMemory(device.device(), vertexBufferMemory, nullptr);
    if(hasIndexBuffer){
        vkDestroyBuffer(device.device(), indexBuffer, nullptr);
        vkFreeMemory(device.device(), indexBufferMemory, nullptr);
    }

}

std::unique_ptr<Model> Model::createModelFromFile(Device& device, const std::string& filepath){
    bufferData bData{};
    bData.loadModel(filepath);
    
    std::cout << "Vertex Count : " << bData.vertices.size() << "\n";
    return std::make_unique<Model>(device, bData);
}

void Model::createVertexBuffers(const std::vector<Vertex>& vertices){
    vertexCount = static_cast<uint32_t>(vertices.size());
    //  assert to check vertexCount is at least 3 (to form basic shape)
    assert(vertexCount >= 3 && "Vertex count must be at least 3!");

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;  //  bufferSize = sizeof 1 vertex * totalnumber
    
    //  previous cpu memory mapping data to cpu writable gpu memory is slow
    //  So we make a staging buffer that takes the cpu data and sends that to GPU specific memory space which is way faster
    VkBuffer stagingBuffer; 
    VkDeviceMemory stagingBufferMemory;
    device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,   //  Tell vullkan this buffer is used as src for memory transfer  
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory );
    //  First copy data to staging buffer
    void* data;
    vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<uint32_t>(bufferSize)); 
    vkUnmapMemory(device.device(), stagingBufferMemory); //  unlink data with gpu data
    
    //  create vertex buffer(Device memory)
    device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,   //    Tell vullkan this buffer is used as dst for memory transfer
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer, vertexBufferMemory);
    //  copy data from staging buffer to Device Memory
    device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    //  After copying we dont need staging buffer anymore so we delete it
    vkDestroyBuffer(device.device(), stagingBuffer, nullptr);   
    vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
}

void Model::createIndexBuffer(const std::vector<uint32_t>& indices){
    indexCount = static_cast<uint32_t>(indices.size());
    hasIndexBuffer = indexCount > 0;    //  true when there is 1 or more index value
    
    if(!hasIndexBuffer){
        return;
    }
    
    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;  

    VkBuffer stagingBuffer; 
    VkDeviceMemory stagingBufferMemory;
    device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,   //  Tell vullkan this buffer is used as src for memory transfer  
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory );

    void* data;
    vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<uint32_t>(bufferSize)); 
    vkUnmapMemory(device.device(), stagingBufferMemory); //  unlink data with gpu data

    device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,  
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer, indexBufferMemory );
        
    //  copy data from staging buffer to Device Memory
    device.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device.device(), stagingBuffer, nullptr);   
    vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
}

void Model::draw(VkCommandBuffer commandBuffer){
    if(hasIndexBuffer){
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }
    else{
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0); 
    }
}

void Model::bind(VkCommandBuffer commandBuffer){
    VkBuffer buffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};   //  starting offset to where binding starts
    //  record to commandBuffer to bind 
    //  [i]th value of "buffer"&"offset" to binding-index["first_binding" + i]
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    if(hasIndexBuffer){
        // VK_INDEX_TYPE should match indices vector type OR represents total vertices that can be represented (2^16-1 || 2^32-1)
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32); 
    }
}

//  INFO:   [Pipeline] Need to provide pipeline with these descriptions -> inside "VkPipelineVertexInputStateCreateInfo"
//  This binding description corresponds to single VertexBuffer
std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions(){
    std::vector<VkVertexInputBindingDescription> bindingDescription(1);
    bindingDescription[0].binding = 0;  //  for binding index "0"
    bindingDescription[0].stride = sizeof(Vertex);  //  read size of Vertex as 1 item => currently bind0 has pos&color data
    bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
//  you can also return 
//  {{0,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, position)},
//  {1,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, position)}}
//  the order is ["location", "binding", "format", "offset"]
}
std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions(){
    std::vector<VkVertexInputAttributeDescription> attributeDescription{};
    //  Order of param      =>      location, binding, format, offset
    attributeDescription.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescription.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescription.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescription.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

    return attributeDescription;
}



void Model::bufferData::loadModel(const std::string &filepath){
    tinyobj::attrib_t attrib;   //  has all the obj data
    std::vector<tinyobj::shape_t> shapes;   //  contains index data for mesh(vertex,index,textureCoord)
    std::vector<tinyobj::material_t> materials; // contains index data for texture(image, albedo,...)
    std::string warn ,err;
    //  LoadObj need components as address to vector, not the fisrt starting address
    //  (which is just the address pointing towards 1 element and no connection with the rest of the container)

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())){
        throw std::runtime_error(warn + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for(const auto &shape : shapes){
        for(const auto &index : shape.mesh.indices){
            Vertex vertex{};
            if(index.vertex_index >= 0){
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };              
                //  attrib.color is filled with 1's + same size as vertex if not existing  
                vertex.color = {
                    //  fetch the color data matching the vertex 
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
                };   
            }
            if(index.normal_index >= 0){
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };                
            }
            if(index.texcoord_index >= 0){
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };                
            }
            //  insert added vertex
            if(uniqueVertices.count(vertex) == 0){
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }
 
}


}