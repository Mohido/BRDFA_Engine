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

    enum BoxSide {
        RIGHT = 0, 
        LEFT = 1, 
        TOP = 2, 
        BOTTOM = 3,
        FRONT = 4,
        BACK = 5
    };


    /// <summary>
    /// Returns the data of one of the skymap sides. 
    /// </summary>
    /// <param name="image"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <param name="face"></param>
    /// <returns></returns>
    char* loadFace(char* image, const int& width, const int& height, const BoxSide& face) {
        int faceOff_x, faceOff_y;
        int faceWidth = width / 4, faceHeight = height / 3;
        int faceSize = faceWidth * faceHeight * 4;


        /*Getting the face offset*/
        switch (face) {
        case FRONT:
            faceOff_x = faceWidth;
            faceOff_y = faceHeight;
            break;
        case BACK:
            faceOff_x = faceWidth*3;
            faceOff_y = faceHeight;
            break;
        case LEFT:
            faceOff_x = 0;
            faceOff_y = faceHeight;
            break;
        case RIGHT:
            faceOff_x = faceWidth * 2;
            faceOff_y = faceHeight;
            break;
        case TOP:
            faceOff_x = faceWidth ;
            faceOff_y = 0;
            break;
        case BOTTOM:
            faceOff_x = faceWidth;
            faceOff_y = faceHeight * 2;
            break;
        default:
            throw std::runtime_error("ERROR: No such a face.");
        }

        
        /* Copying the image data */
        char* imageData = new char[faceSize];
        for (int r = 0; r < faceHeight; r++) {
            int imgY = (faceOff_y + r);
            for (int c = 0; c < faceWidth; c++) {
                int i = c * 4;
                int imgX = (faceOff_x + c) * 4;
                imageData[i + r * faceWidth * 4] = image[imgX + imgY*width*4];
                imageData[i + 1 + r * faceWidth * 4] = image[imgX + imgY*width*4 + 1];
                imageData[i + 2 + r * faceWidth * 4] =  image[imgX + imgY*width * 4 + 2];
                imageData[i + 3 + r * faceWidth * 4] = image[imgX + imgY * width + 3];
            }
        }

        return imageData;
    }


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
        uniformBuffers.resize(swapchain.images.size()*meshCount);       // Mesh Count can be of a maximum size to reduce the allocations
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