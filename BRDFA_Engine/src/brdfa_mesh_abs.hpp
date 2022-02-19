#pragma once
#include "brdfa_structs.hpp"
#include "brdfa_commander_abs.hpp"
#include "brdfa_swap_image_abs.hpp"

#include <string>
#include <iostream>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjectloader/tiny_obj_loader.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>



namespace std {
    template<> struct hash<brdfa::Vertex> {
        size_t operator()(brdfa::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

namespace brdfa {

    /// <summary>
    /// 
    /// </summary>
    /// <param name="mesh"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="modelPath"></param>
    static void loadVertices(Mesh& mesh, Commander& commander, const Device& device, const std::string& modelPath) {
        mesh.vertices.clear();
        mesh.indices.clear();

        /*Reading the model data from file*/
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
            throw std::runtime_error(warn + err);
        }

        /*Transforming the loaded data into mesh data*/
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
                vertex.color = { 1.0f, 1.0f, 1.0f };

                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                    mesh.vertices.push_back(vertex);
                }
                mesh.indices.push_back(uniqueVertices[vertex]);
            }
        }

        /*Creation of Vertex Buffer*/
        /*Creation of staging buffer in RAM*/
        VkDeviceSize bufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();
        Buffer v_staging;
        createBuffer(
            commander, device,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            v_staging);

        /*Filling the RAM memeory with vertices data*/
        void* data;
        vkMapMemory(device.device, v_staging.memory, 0, bufferSize, 0, &data);
        memcpy(data, mesh.vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(device.device, v_staging.memory);

        /*Creation of vertex buffer in GPU RAM*/
        createBuffer(
            commander, device,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT
            | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mesh.vertexBuffer);

        /*Filling the GPU RAM buffer that we just  created.*/
        copyBuffer(
            commander, device,
            v_staging.obj, mesh.vertexBuffer.obj,
            bufferSize);

        /*Clearing the staging Buffer data from RAM.*/
        vkDestroyBuffer(device.device, v_staging.obj, nullptr);
        vkFreeMemory(device.device, v_staging.memory, nullptr);


        /*Creation of Indices Buffer*/
        /*Creation of staging buffer in RAM*/
        bufferSize = sizeof(mesh.indices[0]) * mesh.indices.size();
        Buffer i_staging;
        createBuffer(
            commander, device,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            i_staging);


        /*Filling the staging index buffer*/
        data = nullptr;

        vkMapMemory(device.device, i_staging.memory, 0, bufferSize, 0, &data);
        memcpy(data, mesh.indices.data(), (size_t)bufferSize);
        vkUnmapMemory(device.device, i_staging.memory);

        /*Creation of the Index Buffer in the GPU RAM*/
        createBuffer(
            commander, device,
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT
            | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mesh.indexBuffer);

        /*Copying the Index RAM buffer to index GPU RAM buffer*/
        copyBuffer(
            commander, device,
            i_staging.obj, mesh.indexBuffer.obj,
            bufferSize);

        /*Clearing the index RAM Buffer*/
        vkDestroyBuffer(device.device, i_staging.obj, nullptr);
        vkFreeMemory(device.device, i_staging.memory, nullptr);
    }







    /// <summary>
    /// 
    /// </summary>
    /// <param name="texturePath"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    static void loadTexture(Mesh& mesh, Commander& commander, const Device& device, const std::string& texturePath) {
        if (mesh.textureImages.size() >= 4) 
            throw std::runtime_error("ERROR: We already have 4 textures loaded to this mesh");
        

        mesh.textureImages.push_back({});
        /*Loading Image data from file.*/
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        mesh.textureImages[mesh.textureImages.size()-1].mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        if (!pixels) {
            throw std::runtime_error("ERROR: failed to load texture image!");
        }

        /*Populating the staging buffer in the RAM*/
        Buffer staging;
        createBuffer(
            commander, device,
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging);

        void* data;
        vkMapMemory(device.device, staging.memory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device.device, staging.memory);
        stbi_image_free(pixels);


        /*Creating an empty buffer in the GPU RAM*/
        createImage(
            commander, device,
            texWidth, texHeight,
            mesh.textureImages[mesh.textureImages.size() - 1].mipLevels,
            VK_SAMPLE_COUNT_1_BIT,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            | VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mesh.textureImages[mesh.textureImages.size() - 1]);


        /*Transforming the created Image layout to receive data.*/
        transitionImageLayout(
            mesh.textureImages[mesh.textureImages.size() - 1],
            commander, device,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);


        /*  Filling the Image Buffer in that we just created in the GPU ram.
            Transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps*/
        copyBufferToImage(
            commander, device,
            staging, mesh.textureImages[mesh.textureImages.size() - 1],
            static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));


        /*Cleaning up the staging from the RAM*/
        vkDestroyBuffer(device.device, staging.obj, nullptr);
        vkFreeMemory(device.device, staging.memory, nullptr);


        /*Generating Image mipmaps*/
        generateMipmaps(
            commander, device,
            mesh.textureImages[mesh.textureImages.size() - 1],
            VK_FORMAT_R8G8B8A8_SRGB,
            texWidth, texHeight);


        /*Creating Image view for the texture.*/
        mesh.textureImages[mesh.textureImages.size() - 1].view = createImageView(
            mesh.textureImages[mesh.textureImages.size() - 1].obj, device.device,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT,
            mesh.textureImages[mesh.textureImages.size() - 1].mipLevels);


        /*Create Texture sampler:*/
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device.physicalDevice, &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mesh.textureImages[mesh.textureImages.size() - 1].mipLevels);
        samplerInfo.mipLodBias = 0.0f;

        if (vkCreateSampler(device.device, &samplerInfo, nullptr, &mesh.textureImages[mesh.textureImages.size() - 1].sampler) != VK_SUCCESS) {
            throw std::runtime_error("ERROR: failed to create texture sampler!");
        }
    }





    /// <summary>
    /// 
    /// </summary>
    /// <param name="mesh"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="modelPath"></param>
    /// <param name="texturePath"></param>
    void populate(Mesh& mesh, Commander& commander, const Device& device, const std::string& modelPath, const std::string& texturePath)
    {
        loadVertices(mesh, commander, device, modelPath);
        loadTexture(mesh, commander, device, texturePath);
        vkDeviceWaitIdle(device.device);
    }



	/// <summary>
	/// 
	/// </summary>
	/// <param name="commander"></param>
	/// <param name="device"></param>
	/// <param name="modelPath"></param>
	/// <param name="texturePath"></param>
	/// <returns></returns>
	static Mesh loadMesh(Commander& commander, const Device& device, const std::string& modelPath, const std::string& texturePath) {
        Mesh mesh{};
        populate(mesh, commander, device, modelPath, texturePath);
        return mesh;
	}




    /// <summary>
    /// 
    /// </summary>
    /// <param name="mesh"></param>
    /// <param name="device"></param>
    void destroyMesh(Mesh& mesh, const Device& device) {
        /*Destroying Txture data*/
        for (Image& textureImage : mesh.textureImages) {
            vkDestroySampler(device.device, textureImage.sampler, nullptr);
            vkDestroyImageView(device.device, textureImage.view, nullptr);
            vkDestroyImage(device.device, textureImage.obj, nullptr);
            vkFreeMemory(device.device, textureImage.memory, nullptr);
        }


        /*Destroying Vertices data*/
        vkDestroyBuffer(device.device, mesh.indexBuffer.obj, nullptr);
        vkFreeMemory(device.device, mesh.indexBuffer.memory, nullptr);
        vkDestroyBuffer(device.device, mesh.vertexBuffer.obj, nullptr);
        vkFreeMemory(device.device, mesh.vertexBuffer.memory, nullptr);
    }
}
