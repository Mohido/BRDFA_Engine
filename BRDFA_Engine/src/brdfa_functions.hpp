#pragma once

#include "brdfa_structs.hpp"
#include "brdfa_cons.hpp"

#include "brdfa_instance_abs.hpp"
#include "brdfa_device_abs.hpp"
#include "brdfa_swap_image_abs.hpp"
#include "brdfa_g_pipeline_abs.hpp"
#include "brdfa_commander_abs.hpp"
#include "brdfa_descriptor_abs.hpp"
#include "brdfa_mesh_abs.hpp"
#include "brdfa_callbacks.hpp"

#include <algorithm>
#include <vulkan/vulkan.h>
#include <iostream>


namespace brdfa {

    /// <summary>
    /// 
    /// </summary>
    /// <param name="syncs"></param>
    /// <param name="imagesInFlight"></param>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    /// <param name="maxFramesInFlight"></param>
    static void createSyncObjects( std::vector<SyncCollection>& syncs, std::vector<VkFence>& imagesInFlight, const Device& device, const SwapChain& swapchain, uint8_t maxFramesInFlight) {
        
        syncs.resize(maxFramesInFlight);
        imagesInFlight.resize(swapchain.images.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
         
        for (size_t i = 0; i < maxFramesInFlight; i++) {
            if (vkCreateSemaphore(device.device, &semaphoreInfo, nullptr, &syncs[i].s_imageAvailable) != VK_SUCCESS ||
                vkCreateSemaphore(device.device, &semaphoreInfo, nullptr, &syncs[i].s_renderFinished) != VK_SUCCESS ||
                vkCreateFence(device.device, &fenceInfo, nullptr, &syncs[i].f_inFlight) != VK_SUCCESS) {
                throw std::runtime_error("ERROR: failed to create synchronization objects for a frame!");
            }
        }
    }


    /// <summary>
    /// Creates Empty buffers in the RAM for holding Uniform variables. 
    ///     
    /// Note:
    ///     Each scene object will have its own related uniform buffer.
    ///     Each independent frame will have its own related uniform buffer.
    ///     Therefore, Uniform buffers size is: swapchain images * mesh count.
    /// </summary>
    /// <param name="uniformBuffers"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    /// <param name="meshCount"></param>
    static void createUniformBuffers(std::vector<Buffer>& uniformBuffers, Commander& commander, const Device& device, const SwapChain& swapchain, uint32_t meshCount) {
        VkDeviceSize bufferSize = sizeof(MVPMatrices);       
        uniformBuffers.resize(swapchain.images.size()*meshCount);
        for (size_t i = 0; i < uniformBuffers.size(); i++) {
            createBuffer(
                commander, device,
                bufferSize, 
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
                    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                uniformBuffers[i]);
        }
    }



  
}