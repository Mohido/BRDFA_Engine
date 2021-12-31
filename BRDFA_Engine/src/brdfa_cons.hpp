#pragma once
#include <vulkan/vulkan.h>


//------------------------------------------------ CONSTANTS ------------------------------------------------
static const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};


static const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


const std::string TEXTURE_PATH = "res/textures/viking_room.png";
const std::string MODEL_PATH = "res/objects/viking_room.obj";