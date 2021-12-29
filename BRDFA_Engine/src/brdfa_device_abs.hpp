#pragma once
#include <vulkan/vulkan.h>
#include "brdfa_structs.hpp"
#include "brdfa_cons.hpp"
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <string>
#include <vector>
#include <set>
// --------------------------------- Device Creation Abstraction ---------------------------------


namespace brdfa {


    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <param name="typeFilter"></param>
    /// <param name="properties"></param>
    /// <returns></returns>
    static uint32_t findMemoryType(const Device& device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(device.physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("ERROE: failed to find suitable memory type!");
    }


    /// <summary>
    /// 
    /// </summary>
    /// <param name="physicalDevice"></param>
    /// <returns></returns>
    static VkSampleCountFlagBits getMaxUsableSampleCount(const VkPhysicalDevice& physicalDevice) {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }



    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <param name="surface"></param>
    /// <returns></returns>
    static QueueFamilyIndices findQueueFamilies(const Device& device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device.physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device.physicalDevice, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device.physicalDevice, i, device.surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }



    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    static bool checkDeviceExtensionSupport(const VkPhysicalDevice& device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }




    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    static SwapChainSupportDetails querySwapChainSupport(const Device& device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice, device.surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, device.surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, device.surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, device.surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, device.surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }



    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    static bool isDeviceSuitable(const Device& device) {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device.physicalDevice);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device.physicalDevice, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }


    /// <summary>
    /// 
    /// </summary>
    /// <param name="instance"></param>
    /// <param name="surface"></param>
    /// <param name="device"></param>
    /// <param name="queues"></param>
    static void pickPhysicalDevice(const VkInstance& instance, Device& device) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("ERROR: failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        bool found = false;
        for (const auto& physicalDevice : devices) {
            device.physicalDevice = physicalDevice;
            if (isDeviceSuitable(device)) {
                found = true;
                device.msaaSamples = getMaxUsableSampleCount(physicalDevice);
                break;
            }
        }

        if (!found) {
            throw std::runtime_error("ERROR: failed to find a suitable GPU!");
        }
    }



    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <param name="enableValidationLayers"></param>
    static void createLogicalDevice(Device& device, const bool& enableValidationLayers) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(device.physicalDevice, &createInfo, nullptr, &device.device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device.device, indices.graphicsFamily.value(), 0, &device.graphicsQueue);
        vkGetDeviceQueue(device.device, indices.presentFamily.value(), 0, &device.presentQueue);
    }

}
    