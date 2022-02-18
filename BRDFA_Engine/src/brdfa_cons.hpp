#pragma once
#include <vulkan/vulkan.h>
#include <array>

//------------------------------------------------ CONSTANTS ------------------------------------------------
static const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};


static const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


const std::string TEXTURE_PATH = "res/textures/viking_room.png";
const std::string MODEL_PATH= "res/objects/viking_room.obj";
const std::string  CUBE_MODEL_PATH = "res/objects/cube.obj";




// Order of the skybox...
//  front, back, up, down, right and left
const std::array<std::string, 6> SKYMAP_PATHS = { 
    "res/textures/skybox/right.jpg",
    "res/textures/skybox/left.jpg",
    "res/textures/skybox/top.jpg",
    "res/textures/skybox/bottom.jpg",
    "res/textures/skybox/front.jpg",
    "res/textures/skybox/back.jpg"
};


const std::string SHADERS_PATH = "shaders";
//const std::string SKYMAP_PATHS = "res/textures/skybox_1.png";