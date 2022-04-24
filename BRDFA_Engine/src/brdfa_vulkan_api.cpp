#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

// #include <imgui_text_editor/TextEditor.h>

//#include <string>
//#include <array>
//#include <queue>
//#include <vector>
//#include <thread>
//#include <future>
//#include "brdfa_structs.hpp"
#include <brdfa_vulkan_api.hpp>
#include <brdfa_engine.hpp>
#include <helpers/functions.hpp>
//#include <brdfa_cons.hpp>

namespace brdfa {

	VulkanAPI::~VulkanAPI() {}

	bool VulkanAPI::init() {
		return false;
		//createInstance(APP_NAME.c_str(), m_config.validationLayerEnabled, m_instance);
		//
		//if (glfwCreateWindowSurface(m_instance.instance, m_window, nullptr, &m_device.surface) != VK_SUCCESS) {
		//	throw std::runtime_error("ERROR: failed to create window surface!");
		//}
		///*Initializing the engine.*/
		//pickPhysicalDevice(m_instance.instance, m_device);
		//createLogicalDevice(m_device, m_config.validationLayerEnabled);
		//createSwapChain(m_swapChain, m_device, m_config.width, m_config.height);
		//createRenderPass(m_graphicsPipelines, m_device, m_swapChain);
		//createDescriptorSetLayout(m_descriptorData, m_device, m_swapChain);
		//createCommandPool(m_commander.pool, m_device);
		//createFramebuffers(m_swapChain, m_commander, m_device, m_graphicsPipelines);
		//createSyncObjects(m_sync, m_imagesInFlight, m_device, m_swapChain, m_config.maxFramesInFlight);
	}



	bool  VulkanAPI::render()	{
		return false;
	}



	bool  VulkanAPI::shutdown() {
		return false;
	}



}