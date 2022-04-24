#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include "brdfa_engine.hpp"

//------------------------------------------------ Callbacks functions -------------------------------------

namespace brdfa {
    /// <summary>
    /// Used to enable frame buffer resizing of the window.
    /// </summary>
    /// <param name="window"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    void framebufferResizeCallback(
        GLFWwindow* window, 
        int width, 
        int height);


    void keyCallback(
        GLFWwindow* window, 
        int key, 
        int scancode, 
        int action, 
        int mods);
    


    void mouseButtonCallback(
        GLFWwindow* window, 
        int button, 
        int action, 
        int mods);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="messageSeverity"></param>
    /// <param name="messageType"></param>
    /// <param name="pCallbackData"></param>
    /// <param name="pUserData"></param>
    /// <returns></returns>
     VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

}