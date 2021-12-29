#pragma once

#include "brdfa_structs.hpp"
#include "brdfa_cons.hpp"

#include <algorithm>
#include <vulkan/vulkan.h>
#include <iostream>

namespace brdfa {




    static void createDescriptorSetLayout(Descriptor& descriptorObj, const Device& device, const SwapChain& swapchain) {

        /*Uniform variables.*/
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

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
    static void initDescriptors(Descriptor& descriptorObj, const Device& device, const SwapChain& swapchain, const std::vector<Buffer>& uniformBuffers, std::vector<Mesh>& meshes) {

        /*Descriptor Pool creation*/
        size_t descriptorCount = swapchain.images.size() * meshes.size(); // How many descriptors of this kind can be allocated through the whole sets
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = descriptorCount;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = descriptorCount;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = descriptorCount; // we want to allocate a set for each object.

        if (vkCreateDescriptorPool(device.device, &poolInfo, nullptr, &descriptorObj.pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }


        ///*Allocating descriptor sets*/
        size_t setCount = descriptorCount;// swapchain.images.size() * meshCount;
        std::vector<VkDescriptorSetLayout> layouts(setCount, descriptorObj.layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorObj.pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(setCount);
        allocInfo.pSetLayouts = layouts.data();

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


            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorObj.sets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorObj.sets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, VK_NULL_HANDLE);
        }
    }

}
