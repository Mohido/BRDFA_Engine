#pragma once
#include<brdfa_render_api.hpp>

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN



#include <array>
#include <vector>
#include "brdfa_structs.hpp"

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

namespace brdfa {
	class VulkanAPI : public RenderAPI {
	public:
		VulkanAPI(const RenderAPIConfig& config, GLFWwindow* window)
			: RenderAPI(config, RenderAPIType::RENDER_API_VULKAN), m_window(window)
		{}

		~VulkanAPI() override;
		bool init() override;			// Initialize the API
		bool render() override;			// A render call
		bool shutdown() override;		// Shuts down the API
		bool loadPipelines() {};		// Load the pipelines

	private:
		/*Vulkan objects*/
		Instance										m_instance;                     // Vulkan process handler
		Device											m_device;                       // Holds the physical/logical device of the engine.
		SwapChain										m_swapChain;                    // swapchain related objects.
		Descriptor										m_descriptorData;               // Holds Descriptor pool and its relative layout and sets.
		GPipeline										m_graphicsPipelines;				// Holds the Graphics pipeline data.
		Commander										m_commander;					// Handles the command pool and its related command buffers.
		std::vector<SyncCollection>						m_sync;							// Fences per swapchain image. CPU/GPU signals, Semaphores per swapchain image. GPU/GPU signals.
		std::vector<VkFence>							m_imagesInFlight;				// 

		/*GLFW Objects*/
		GLFWwindow*										m_window;						// A pointer to the window to render into.
	};
}

