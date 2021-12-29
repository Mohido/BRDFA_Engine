#pragma once
#include <vulkan/vulkan.h>


#include "brdfa_engine.hpp"
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>


#include "brdfa_functions.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>


#include <iostream>
#include <chrono>




const std::string TEXTURE_PATH = "res/textures/viking_room.png";
const std::string MODEL_PATH = "res/objects/viking_room.obj";


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



	void BRDFA_Engine::close() {

		cleanup();

		for (auto& mesh : m_meshes) {
			destroyMesh(mesh, m_device);
		}

		/*destroying objects data.*/
		m_meshes.clear();

		vkDestroyDescriptorSetLayout(m_device.device, m_descriptorData.layout, nullptr);

		/*destorying Engine related stuff.*/
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_device.device, m_sync[i].s_renderFinished, nullptr);
			vkDestroySemaphore(m_device.device, m_sync[i].s_imageAvailable, nullptr);
			vkDestroyFence(m_device.device, m_sync[i].f_inFlight, nullptr);
		}

		vkDestroyCommandPool(m_device.device, m_commander.pool, nullptr);
		vkDestroyDevice(m_device.device, nullptr);

		/*destroying bebug util massenger.*/
		if (m_configuration.validationLayersEnabled) {
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance.instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr) {
				func(m_instance.instance, m_instance.debugMessenger, nullptr);
			}
		}

		vkDestroySurfaceKHR(m_instance.instance, m_device.surface, nullptr);
		vkDestroyInstance(m_instance.instance, nullptr);

		glfwDestroyWindow(m_window);
		glfwTerminate();
		m_active = false;
	}


	/// <summary>
	/// 
	/// </summary>
	BRDFA_Engine::~BRDFA_Engine() {
		if (m_active) {
			close();
			std::cout << "Please, next time make sure to close the engine before forcly shutting the program." << std::endl;
		}
	}






	/// <summary>
	/// Starts the engine. 
	/// </summary>
	/// <returns>True if the engine started successfully and false otherwise.</returns>
	bool BRDFA_Engine::init() {
		try{
			startWindow();
			startVulkan();
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
			return false;
		}
		return true;
	}



	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	bool BRDFA_Engine::interrupt() {
		return true;
	}




	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	bool BRDFA_Engine::loadObject() {
		return true;
	}


	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	bool BRDFA_Engine::updateAndRender() {
		if(glfwWindowShouldClose(m_window)) {
			m_active = false;
			return false;
		}
		glfwPollEvents();
		
		vkWaitForFences(m_device.device, 1, &m_sync[m_currentFrame].f_inFlight, VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(
			m_device.device, 
			m_swapChain.swapChain,
			UINT64_MAX, 
			m_sync[m_currentFrame].s_imageAvailable,
			VK_NULL_HANDLE, 
			&imageIndex);


		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate();
			return true;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("ERROR: failed to acquire swap chain image!");
		}

		update(imageIndex);

		if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_device.device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}
		m_imagesInFlight[imageIndex] = m_sync[m_currentFrame].f_inFlight;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_sync[m_currentFrame].s_imageAvailable};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commander.buffers[imageIndex];

		VkSemaphore signalSemaphores[] = { m_sync[m_currentFrame].s_renderFinished};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_device.device, 1, &m_sync[m_currentFrame].f_inFlight);

		if (vkQueueSubmit(m_device.graphicsQueue, 1, &submitInfo, m_sync[m_currentFrame].f_inFlight) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_swapChain.swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(m_device.presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_frameBufferResized) {
			m_frameBufferResized = false;
			recreate();
		}

		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		return true;
	}


	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	bool BRDFA_Engine::isClosed() {
		return !m_active;
	}




	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	bool BRDFA_Engine::frameBufferResize()
	{
		this->m_frameBufferResized = true;
		return this->m_frameBufferResized;
	}

// ------------------------------------------------ MEMBER FUNCTIONS ---------------------------------------


	void BRDFA_Engine::startWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		m_window = glfwCreateWindow(m_configuration.width, m_configuration.height, "BRDFA Engine", nullptr, nullptr);
		if (!m_window) {
			throw std::runtime_error("ERROR: can't open a window!");
		}

		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

		glfwGetFramebufferSize(m_window, reinterpret_cast<int*>(&m_width_w), reinterpret_cast<int*>(&m_height_w));
	}





	void BRDFA_Engine::startVulkan()
	{
		createInstance("BRDFA Engine", m_configuration.validationLayersEnabled, m_instance);

		if (glfwCreateWindowSurface(m_instance.instance, m_window, nullptr, &m_device.surface) != VK_SUCCESS) {
			throw std::runtime_error("ERROR: failed to create window surface!");
		}

		/*Initializing the engine.*/
		pickPhysicalDevice(m_instance.instance, m_device);
		createLogicalDevice(m_device, m_configuration.validationLayersEnabled);
		createSwapChain(m_swapChain, m_device, m_width_w, m_height_w);
		createRenderPass(m_graphicsPipeline, m_device, m_swapChain);
		createDescriptorSetLayout(m_descriptorData, m_device, m_swapChain);
		createGraphicsPipeline(m_graphicsPipeline, m_device, m_swapChain, m_descriptorData);
		createCommandPool(m_commander.pool, m_device);
		createFramebuffers(m_swapChain, m_commander, m_device, m_graphicsPipeline);
		createSyncObjects(m_sync, m_imagesInFlight, m_device, m_swapChain, MAX_FRAMES_IN_FLIGHT);
		
		//// Model dependent functions...
		/*Initializing Mesh Related functionalities.*/
		m_meshes.push_back(loadMesh(m_commander, m_device, MODEL_PATH, TEXTURE_PATH ));
		m_meshes.push_back(loadMesh(m_commander, m_device, MODEL_PATH, TEXTURE_PATH));
		createUniformBuffers(m_uniformBuffers, m_commander, m_device, m_swapChain, m_meshes.size());
		initDescriptors(m_descriptorData, m_device, m_swapChain, m_uniformBuffers, m_meshes);
		recordCommandBuffers(m_commander, m_device, m_graphicsPipeline, m_descriptorData, m_swapChain, m_meshes);

		/*Engine is ready!*/
		m_active = true;
	}




	/// <summary>
	/// 
	/// </summary>
	void BRDFA_Engine::cleanup() {
		vkDeviceWaitIdle(m_device.device);

		/*Clearing up the swapchain resources.*/
		/*clearn the depth Image buffer*/
		vkDestroyImageView(m_device.device, m_swapChain.depthImage.view, nullptr);
		vkDestroyImage(m_device.device, m_swapChain.depthImage.obj, nullptr);
		vkFreeMemory(m_device.device, m_swapChain.depthImage.memory, nullptr);
		/*clearn the color Image buffer*/
		vkDestroyImageView(m_device.device, m_swapChain.colorImage.view, nullptr);
		vkDestroyImage(m_device.device, m_swapChain.colorImage.obj, nullptr);
		vkFreeMemory(m_device.device, m_swapChain.colorImage.memory, nullptr);
		/*Clearing framebuffers*/
		for (auto framebuffer : m_swapChain.framebuffers) {
			vkDestroyFramebuffer(m_device.device, framebuffer, nullptr);
		}


		/*Deleting all the current allocated command buffers*/
		vkFreeCommandBuffers(m_device.device, m_commander.pool, static_cast<uint32_t>(m_commander.buffers.size()), m_commander.buffers.data());

		/*Clearing the Graphics pipeline*/
		vkDestroyPipeline(m_device.device, m_graphicsPipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(m_device.device, m_graphicsPipeline.layout, nullptr);
		vkDestroyRenderPass(m_device.device, m_graphicsPipeline.renderPass, nullptr);

		/*Delete the remaining swapchain objects*/
		for (auto imageView : m_swapChain.imageViews) {
			vkDestroyImageView(m_device.device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_device.device, m_swapChain.swapChain, nullptr);

		/*Clearing the Objects related data to recreate them.*/
		/*Deleting the uniform buffers.*/
		for (size_t i = 0; i < m_swapChain.images.size()*m_meshes.size(); i++) {
			vkDestroyBuffer(m_device.device, m_uniformBuffers[i].obj, nullptr);
			vkFreeMemory(m_device.device, m_uniformBuffers[i].memory, nullptr);
		}
		

		vkDestroyDescriptorPool(m_device.device, m_descriptorData.pool, nullptr);
	}




	



	/// <summary>
	/// 
	/// </summary>
	void BRDFA_Engine::recreate() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(m_window, &width, &height);
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(m_device.device);

		cleanup();

		/*Vulkan Re-initialization.*/
		createSwapChain(m_swapChain, m_device, m_width_w, m_height_w);
		createRenderPass(m_graphicsPipeline, m_device, m_swapChain);
		createDescriptorSetLayout(m_descriptorData, m_device, m_swapChain);
		createGraphicsPipeline(m_graphicsPipeline, m_device, m_swapChain, m_descriptorData);
		createFramebuffers(m_swapChain, m_commander, m_device, m_graphicsPipeline);

		/*Meshes dependent*/
		createUniformBuffers(m_uniformBuffers, m_commander, m_device, m_swapChain, m_meshes.size());
		initDescriptors(m_descriptorData, m_device, m_swapChain, m_uniformBuffers, m_meshes);
		recordCommandBuffers(m_commander, m_device, m_graphicsPipeline, m_descriptorData, m_swapChain, m_meshes);

		/*Syncronization objects re-initialization.*/
		m_imagesInFlight.resize(m_swapChain.images.size(), VK_NULL_HANDLE);
	}




	/// <summary>
	/// 
	/// </summary>
	/// <param name="currentImage"></param>
	void BRDFA_Engine::update(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		for (size_t i = 0; i < m_meshes.size(); i++) {

			size_t ind = i * m_swapChain.images.size() + currentImage;
			MVPMatrices ubo{};
			glm::mat4 modelTr = glm::mat4(1.0f);
			modelTr[3][0] = 1.0f*i;

			ubo.model = glm::rotate(modelTr, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj = glm::perspective(glm::radians(45.0f), m_swapChain.extent.width / (float)m_swapChain.extent.height, 0.1f, 10.0f);
			ubo.proj[1][1] *= -1;

			void* data;
			vkMapMemory(m_device.device, m_uniformBuffers[ind].memory, 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(m_device.device, m_uniformBuffers[ind].memory);
		}
		
	}
}
