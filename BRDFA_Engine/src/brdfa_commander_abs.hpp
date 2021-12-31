#pragma once

#include "brdfa_structs.hpp"
#include "brdfa_cons.hpp"

#include <algorithm>
#include <vulkan/vulkan.h>
#include <iostream>



namespace brdfa {




    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    static void endSingleTimeCommands(Commander& commander, const Device& device) {
        VkCommandBuffer commandBuffer = commander.sceneBuffers.back();
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.graphicsQueue);

        vkFreeCommandBuffers(device.device, commander.pool, 1, &commandBuffer);
        commander.sceneBuffers.pop_back();
    }




    /// <summary>
    /// The commander buffers will act as a stack in this function.
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <returns></returns>
    static VkCommandBuffer beginSingleTimeCommands(Commander& commander, const Device& device) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commander.pool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device.device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        commander.sceneBuffers.push_back(commandBuffer);
        return commandBuffer;
    }



    /// <summary>
    /// 
    /// </summary>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <param name="mipLevels"></param>
    /// <param name="numSamples"></param>
    /// <param name="format"></param>
    /// <param name="tiling"></param>
    /// <param name="usage"></param>
    /// <param name="properties"></param>
    /// <param name="image"></param>
    /// <param name="imageMemory"></param>
    static void createImage(const Commander& commander, const Device& device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, Image& image) {
        
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device.device, &imageInfo, nullptr, &image.obj) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device.device, image.obj, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(device, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device.device, &allocInfo, nullptr, &image.memory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        image.width = width;
        image.height = height;
        image.mipLevels = mipLevels;
        vkBindImageMemory(device.device, image.obj, image.memory, 0);
    }




    /// <summary>
    /// 
    /// </summary>
    /// <param name="size"></param>
    /// <param name="usage"></param>
    /// <param name="properties"></param>
    /// <param name="buffer"></param>
    /// <param name="bufferMemory"></param>
    static void  createBuffer(Commander& commander, const Device& device , VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, Buffer& buffer) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device.device, &bufferInfo, nullptr, &buffer.obj) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device.device, buffer.obj, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(device, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device.device, &allocInfo, nullptr, &buffer.memory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device.device, buffer.obj, buffer.memory, 0);
    }




    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="srcBuffer"></param>
    /// <param name="dstBuffer"></param>
    /// <param name="size"></param>
    static void copyBuffer(Commander& commander, const Device& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(commander, device);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commander, device);
    }





    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="buffer"></param>
    /// <param name="image"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    static void copyBufferToImage(Commander& commander, const Device& device, Buffer& buffer, Image& image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(commander, device);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer.obj, image.obj, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(commander, device);
    }


    /// <summary>
    /// 
    /// </summary>
    /// <param name="commandPool"></param>
    /// <param name="device"></param>
    static void createCommandPool(VkCommandPool& commandPool, const Device& device) {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device.device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }



    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="gpipeline"></param>
    /// <param name="swapchain"></param>
    /// <param name="index"></param>
    static void updateUICommandBuffers(Commander& commander, const Device& device, const GPipeline& gpipeline, const SwapChain& swapchain, const uint32_t index) {

        if (commander.uiBuffers.size() != swapchain.images.size()) {
            throw std::runtime_error("ERROR: You must intialize the commander Scene buffer first by calling createCommandBuffers() function.");
        }

        /*deleting the existing buffer from the pool*/
        vkFreeCommandBuffers(device.device, commander.pool, 1, &commander.uiBuffers[index]);

        /*Allocate the commandbuffer to the pool*/
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commander.pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(device.device, &allocInfo, &commander.uiBuffers[index]) != VK_SUCCESS) {
            throw std::runtime_error("ERROR: failed to allocate command buffers!");
        }


        /*Begin the command buffer recording.*/
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(commander.uiBuffers[index], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("ERROR: failed to begin recording command buffer!");
        }

        /*Render pass begins*/
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = gpipeline.uiRenderpass;
        renderPassInfo.framebuffer = swapchain.framebuffers[index];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapchain.extent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); 
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commander.uiBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commander.uiBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, gpipeline.pipeline);

        /*Drawing data to the framebuffer*/
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData)
        {
            drawData->DisplayPos = { 0, 0 };
            ImGui_ImplVulkan_RenderDrawData(drawData, commander.uiBuffers[index]);
        }

        vkCmdEndRenderPass(commander.uiBuffers[index]);
        if (vkEndCommandBuffer(commander.uiBuffers[index]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }


    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="gpipeline"></param>
    /// <param name="descriptorObj"></param>
    /// <param name="swapchain"></param>
    /// <param name="meshes"></param>
    static void recordCommandBuffers(Commander& commander, const Device& device, const GPipeline& gpipeline, const Descriptor& descriptorObj ,const SwapChain& swapchain, std::vector<Mesh>& meshes) {
        commander.sceneBuffers.resize(swapchain.framebuffers.size());
        commander.uiBuffers.resize(swapchain.framebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commander.pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commander.sceneBuffers.size();

        if (vkAllocateCommandBuffers(device.device, &allocInfo, commander.sceneBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("ERROR: failed to allocate command buffers!");
        }

        for (size_t i = 0; i < commander.sceneBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
           

            if (vkBeginCommandBuffer(commander.sceneBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("ERROR: failed to begin recording command buffer!");
            }
            /*IMGUI: */
            //ImGui_ImplVulkan_CreateFontsTexture(commander.buffers[i]);


            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = gpipeline.sceneRenderPass;
            renderPassInfo.framebuffer = swapchain.framebuffers[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = swapchain.extent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clearValues[1].depthStencil = { 1.0f, 0 };

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commander.sceneBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commander.sceneBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gpipeline.pipeline);

            for (size_t j = 0; j < meshes.size(); j++) {

                /*Get buffer address from the device:*/

                VkBuffer vertexBuffers[] = { meshes[j].vertexBuffer.obj };
                VkDeviceSize offsets[] = { 0 };
                
                
                vkCmdBindVertexBuffers(commander.sceneBuffers[i], 0, 1, vertexBuffers, offsets);

                vkCmdBindIndexBuffer(commander.sceneBuffers[i], meshes[j].indexBuffer.obj, 0, VK_INDEX_TYPE_UINT32);

                int descriptorSetIndex = j * swapchain.images.size() + i;
                vkCmdBindDescriptorSets(commander.sceneBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gpipeline.layout, 0, 1, &descriptorObj.sets[descriptorSetIndex], 0, nullptr);

                vkCmdDrawIndexed(commander.sceneBuffers[i], meshes[j].indices.size(), 1, 0, 0, 0);
            }
           
            // This can't be implemented in a prerecorded commadn buffer
            //
            vkCmdEndRenderPass(commander.sceneBuffers[i]);
            if (vkEndCommandBuffer(commander.sceneBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }
}