#pragma once

#include "brdfa_structs.hpp"
#include "brdfa_cons.hpp"

#include <algorithm>
#include <vulkan/vulkan.h>
#include <iostream>



#define TEXTURE_COUNT 4



namespace brdfa {



    /// <summary>
    /// 
    /// </summary>
    /// <param name="descriptorObj"></param>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    static void createDescriptorSetLayout(Descriptor& descriptorObj, const Device& device, const SwapChain& swapchain) {

        /*Uniform variables.*/
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        /*Sky map binding*/
        VkDescriptorSetLayoutBinding skymapLayoutBinding{};
        skymapLayoutBinding.binding = 1;
        skymapLayoutBinding.descriptorCount = 1;
        skymapLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        skymapLayoutBinding.pImmutableSamplers = nullptr;
        skymapLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding iTextureLayoutBinding1{};
        iTextureLayoutBinding1.binding = 2;
        iTextureLayoutBinding1.descriptorCount = 1;
        iTextureLayoutBinding1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        iTextureLayoutBinding1.pImmutableSamplers = nullptr;
        iTextureLayoutBinding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding iTextureLayoutBinding2{};
        iTextureLayoutBinding2.binding = 3;
        iTextureLayoutBinding2.descriptorCount = 1;
        iTextureLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        iTextureLayoutBinding2.pImmutableSamplers = nullptr;
        iTextureLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding iTextureLayoutBinding3{};
        iTextureLayoutBinding3.binding = 4;
        iTextureLayoutBinding3.descriptorCount = 1;
        iTextureLayoutBinding3.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        iTextureLayoutBinding3.pImmutableSamplers = nullptr;
        iTextureLayoutBinding3.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding iTextureLayoutBinding4{};
        iTextureLayoutBinding4.binding = 5;
        iTextureLayoutBinding4.descriptorCount = 1;
        iTextureLayoutBinding4.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        iTextureLayoutBinding4.pImmutableSamplers = nullptr;
        iTextureLayoutBinding4.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 6> bindings = { 
            uboLayoutBinding, 
            skymapLayoutBinding, 
            iTextureLayoutBinding1, iTextureLayoutBinding2, iTextureLayoutBinding3, iTextureLayoutBinding4 };

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device.device, &layoutInfo, nullptr, &descriptorObj.layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }



   /// <summary>
   /// creates a BRDFA Descriptor Object. 
   /// Descriptor Object holds the sets being created, the pool which they correspond to, and
   /// their layout in memory. Number of sets is determined by: swapchain images size * meshes count in the scene.
   /// </summary>
   /// <param name="descriptorObj">The desired Descriptor Object to be filled</param>
   /// <param name="device">BRDFA Device object</param>
   /// <param name="swapchain">BRDFA SwapChain object</param>
   /// <param name="meshCount">Number of meshes needed to be rendered</param>
    static void initDescriptors(Descriptor& descriptorObj, const Device& device, const SwapChain& swapchain, const std::vector<Buffer>& uniformBuffers, std::vector<Mesh>& meshes, Image& skymap) {

        /*Descriptor Pool creation*/
        size_t descriptorCount = swapchain.images.size() * meshes.size(); // How many descriptors of this kind can be allocated through the whole sets
        std::array<VkDescriptorPoolSize, 3> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;          // ubov
        poolSizes[0].descriptorCount = descriptorCount;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;  // texture
        poolSizes[1].descriptorCount = descriptorCount * TEXTURE_COUNT;
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;  // environment map
        poolSizes[2].descriptorCount = descriptorCount;


        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = descriptorCount; // we want to allocate a set for each object.

        if (vkCreateDescriptorPool(device.device, &poolInfo, nullptr, &descriptorObj.pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }


        /*Allocating descriptor sets*/
        size_t setCount = descriptorCount;          // swapchain.images.size() * meshCount;
        std::vector<VkDescriptorSetLayout> layouts(setCount, descriptorObj.layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorObj.pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(setCount);
        allocInfo.pSetLayouts = layouts.data();

        // Every 2 consecutive sets are for an individual object in that frame.
        descriptorObj.sets.resize(setCount);
        if (vkAllocateDescriptorSets(device.device, &allocInfo, descriptorObj.sets.data()) != VK_SUCCESS) {
            throw std::runtime_error("ERROR: failed to allocate descriptor sets!");
        }


        for (size_t i = 0; i < descriptorObj.sets.size(); i++) {
            Image texture = meshes[i / swapchain.images.size()].textureImage;

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i].obj;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(MVPMatrices);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture.view;
            imageInfo.sampler = texture.sampler;

            VkDescriptorImageInfo skymapInfo{};
            skymapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            skymapInfo.imageView = skymap.view;
            skymapInfo.sampler = skymap.sampler;

            std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
            /*Mesh UBO uniform*/
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorObj.sets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            
            /*Mesh Texture uniform*/
            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorObj.sets[i];
            descriptorWrites[1].dstBinding = 2;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            /*Mesh Skymap uniform*/
            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = descriptorObj.sets[i];
            descriptorWrites[2].dstBinding = 1;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pImageInfo = &skymapInfo;

            vkUpdateDescriptorSets(device.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, VK_NULL_HANDLE);
        }
    }

}
