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
    void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<BRDFA_Engine*>(glfwGetWindowUserPointer(window));
        app->frameBufferResize();
    }



    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<BRDFA_Engine*>(glfwGetWindowUserPointer(window));
       //if(action == GLFW_REPEAT || action == GLFW_PRESS)

        app->fireKeyEvent(key, action);
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="messageSeverity"></param>
    /// <param name="messageType"></param>
    /// <param name="pCallbackData"></param>
    /// <param name="pUserData"></param>
    /// <returns></returns>
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {

        
        switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return VK_FALSE;
            std::cout << "VALIDATION LAYER: ";
            std::cout << "INFO: ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return VK_FALSE;
            std::cout << "VALIDATION LAYER: ";
            std::cout << "INFO: ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            std::cout << "VALIDATION LAYER: ";
            std::cout << "WARNING: ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            std::cout << "VALIDATION LAYER: ";
            std::cout << "ERROR: ";
            break;
        default:
            std::cout << "VALIDATION LAYER: ";
            std::cout << "UNKOWN SEVERITY: ";
            break;
        }
        std::cout << pCallbackData->pMessage << std::endl;
        std::cout << std::endl;
        return VK_FALSE;
    }

}