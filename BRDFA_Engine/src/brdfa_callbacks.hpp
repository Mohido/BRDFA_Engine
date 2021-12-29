#pragma once
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>


//------------------------------------------------ Callbacks functions -------------------------------------

namespace brdfa {
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