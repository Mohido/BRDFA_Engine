#pragma once

// IMGUI dependencies
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

// Vulkan dependencies
#include <vulkan/vulkan.h>

// GLFW Dependencies
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

// GLM Dependencies
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

// BRDFA_Engine Dependencies
#include "brdfa_engine.hpp"
#include "brdfa_cons.hpp"
#include "brdfa_functions.hpp"
#include "brdfa_callbacks.hpp"

// STD Dependencies
#include <iostream>
#include <chrono>




namespace brdfa {
	
	
	/// <summary>
	/// Destroys the engine if it is still active when exiting the engine scope.
	/// </summary>
	BRDFA_Engine::~BRDFA_Engine() {
		if (m_active) {
			close();
			std::cout << "Please, next time make sure to close the engine before forcly shutting the program." << std::endl;
		}
	}


	/// <summary>
	/// Starts the engine. Initializes teh window, vulkan and imgui.
	/// </summary>
	/// <returns>True: if the engine started successfully. Otherwise, false.</returns>
	bool BRDFA_Engine::init() {
		try {
			startWindow();
			startVulkan();
			startImgui();
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
			return false;
		}
		return true;
	}


	/// <summary>
	/// Update and render call. Returns if it updates and rendered successfully. 
	/// If it returns false it means that the engine has been closed.
	/// </summary>
	/// <returns>if successfully updated and rendered</returns>
	bool BRDFA_Engine::updateAndRender() {
		/*if engine was closed.*/
		if (glfwWindowShouldClose(m_window)) {
			m_active = false;
			return false;
		}
		glfwPollEvents();


		/*Waiting for the images in flight*/
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
		render(imageIndex);

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		
		return true;
	}


	/// <summary>
	/// Shuts down the engine. Called after done using the engine. Closing the window of the engine will close it automatically. 
	/// Therefore, you don't need to call this function unless you want to close the engine by having a callback functionality or whatsoever.
	/// </summary>
	void BRDFA_Engine::close() {
		ImGui_ImplVulkan_DestroyFontUploadObjects();
		vkDestroyDescriptorPool(m_device.device, m_imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		cleanup();
		/* Clear the Meshes*/
		for (auto& mesh : m_meshes) { destroyMesh(mesh, m_device); }
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

		/* destroying bebug util massenger.*/
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
	/// Saved for future usage.
	/// </summary>
	/// <returns>true</returns>
	bool BRDFA_Engine::interrupt() {
		return true;
	}


	/// <summary>
	/// Used to load objects dynamically into the engine. 
	/// </summary>
	/// <returns>If object is loaded successfully</returns>
	bool BRDFA_Engine::loadObject() {
		return true;
	}



	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	bool BRDFA_Engine::loadEnvironmentMap(const std::array<std::string, 6>& skyboxSides) {
		/*Loading Image data from file.*/
		int texWidth, texHeight, texChannels;
		stbi_uc* textureData[6];
		for (int i = 0; i < 6; i++) {
			textureData[i] = stbi_load(skyboxSides[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			if (!textureData[i]) {
				throw std::runtime_error("ERROR: failed to load Environment Map image!: " + skyboxSides[i] + " Does not exist!");
			}
		}
		
		VkDeviceSize imageSize = texWidth * texHeight * 4 * 6;				// Full buffer size (The size of all the images.)
		VkDeviceSize layerSize = imageSize / 6;								// Size per layer

		/*Populating the staging buffer in the RAM*/
		Buffer staging;
		createBuffer(
			m_commander, m_device,
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging);


		/*Copying into the Staging buffer.*/
		void* data;
		vkMapMemory(m_device.device, staging.memory, 0, imageSize, 0, &data);
		for (int i = 0; i < 6; i++) {
			memcpy(static_cast<char*>(data) + (layerSize*i), textureData[i], static_cast<size_t>(layerSize));
		}
		vkUnmapMemory(m_device.device, staging.memory);
		
		/*Freeing the Loaded data in the RAM*/
		for (int i = 0; i < 6; i++) {
			stbi_image_free(textureData[i]);
		}
		

		/*Creating an empty buffer in the GPU RAM*/
		createImage(
			m_commander, m_device,
			texWidth, texHeight,
			1, VK_SAMPLE_COUNT_1_BIT,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_skymap, true);


		/*Transforming the created Image layout to receive data.*/
		transitionImageLayout(
			m_skymap,
			m_commander, m_device,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);


		/*  Filling the Image Buffer in that we just created in the GPU ram.
			Transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps*/
		copyBufferToImage(
			m_commander, m_device,
			staging, m_skymap,
			static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

		//
		/*Cleaning up the staging from the RAM*/
		vkDestroyBuffer(m_device.device, staging.obj, nullptr);
		vkFreeMemory(m_device.device, staging.memory, nullptr);

		/*Transforming the image layout to shader read bit*/
		transitionImageLayout(
			m_skymap,
			m_commander, m_device,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


		/*Creating sky map image view for the skymap.*/
		m_skymap.view = createImageView(
			m_skymap.obj, m_device.device,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_skymap.mipLevels, true);


		/*Create Texture sampler:*/
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_device.physicalDevice, &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(m_skymap.mipLevels);
		samplerInfo.mipLodBias = 0.0f;

		if (vkCreateSampler(m_device.device, &samplerInfo, nullptr, &m_skymap.sampler) != VK_SUCCESS) {
			throw std::runtime_error("ERROR: failed to create texture sampler!");
		}
	}


	/// <summary>
	/// checks if the engine is still active or closed.
	/// </summary>
	/// <returns>if engine is closed</returns>
	bool BRDFA_Engine::isClosed() {
		return !m_active;
	}


	/// <summary>
	/// frame buffer resize signal. It is an internal signal to update the engine window size and its contents.
	/// </summary>
	/// <returns>the resized signal</returns>
	bool BRDFA_Engine::frameBufferResize()
	{
		this->m_frameBufferResized = true;
		return this->m_frameBufferResized;
	}




	void BRDFA_Engine::fireKeyEvent(int key, int action) {
		/*checking for Shifts*/
		m_keyboardEvent = {key, action};

		/*if (action != GLFW_RELEASE) {
			m_keyboardEvent.key_shift = key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT;
			m_keyboardEvent.key_w = key == GLFW_KEY_W;
			m_keyboardEvent.key_s = key == GLFW_KEY_S;
			m_keyboardEvent.key_a = key == GLFW_KEY_A;
			m_keyboardEvent.key_d = key == GLFW_KEY_D;
		}*/
	}

// ------------------------------------------------ MEMBER FUNCTIONS ---------------------------------------

	/// <summary>
	/// Starts the GLFW window.
	/// </summary>
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
		glfwSetKeyCallback(m_window, keyCallback);

		glfwGetFramebufferSize(m_window, reinterpret_cast<int*>(&m_width_w), reinterpret_cast<int*>(&m_height_w));
	}

	
	/// <summary>
	/// Initializes the vulkan engine.
	/// </summary>
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

		/*Initializing Mesh Related functionalities.*/
		m_meshes.push_back(loadMesh(m_commander, m_device, MODEL_PATH, TEXTURE_PATH ));
		m_camera = Camera(m_swapChain.extent.width, m_swapChain.extent.height, 0.1f, 10.0f, 45.0f);
		loadEnvironmentMap(SKYMAP_PATHS);


		createUniformBuffers(m_uniformBuffers, m_commander, m_device, m_swapChain, m_meshes.size());
		initDescriptors(m_descriptorData, m_device, m_swapChain, m_uniformBuffers, m_meshes, m_skymap);
		
		/*Recording the command buffers.*/
		recordCommandBuffers(m_commander, m_device, m_graphicsPipeline, m_descriptorData, m_swapChain, m_meshes);

		/*Engine is ready!*/
		m_active = true;
	}


	/// <summary>
	/// 
	/// </summary>
	/// <returns></returns>
	bool BRDFA_Engine::startImgui() {
		//1: create descriptor pool for IMGUI
		// the size of the pool is very oversize, but it's copied from imgui demo itself.
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		if (vkCreateDescriptorPool(m_device.device, &pool_info, nullptr, &m_imguiPool)) {
			throw std::runtime_error("ERROR: can not create IMGUI descriptor pool!");
		}

		// 2: initialize imgui library
		//this initializes the core structures of imgui
		ImGui::CreateContext();

		//this initializes imgui for GLFW_VULKAN
		bool initGLFW_V = ImGui_ImplGlfw_InitForVulkan(m_window, true);
		QueueFamilyIndices families = findQueueFamilies(m_device);

		//this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = m_instance.instance;
		init_info.PhysicalDevice = m_device.physicalDevice;
		init_info.Device = m_device.device;
		init_info.Queue = m_device.graphicsQueue;
		init_info.DescriptorPool = m_imguiPool;
		init_info.MinImageCount = m_swapChain.images.size();
		init_info.ImageCount = m_swapChain.images.size();
		init_info.MSAASamples = m_device.msaaSamples;

		bool initV = ImGui_ImplVulkan_Init(&init_info, m_graphicsPipeline.sceneRenderPass);

		//execute a gpu command to upload imgui font textures
		VkCommandBuffer cmd = beginSingleTimeCommands(m_commander, m_device);
		bool createTV = ImGui_ImplVulkan_CreateFontsTexture(cmd);
		endSingleTimeCommands(m_commander, m_device);

		return true;
	}


	/// <summary>
	/// private member function to update the engine variables. It is time dependent and frames Independent.
	/// </summary>
	/// <param name="currentImage">The image Index currently being processed. (Inflag image)</param>
	void BRDFA_Engine::update(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		static auto lastTime = startTime;
		//static auto upsTime = startTime;

		auto currentTime = std::chrono::high_resolution_clock::now();
		
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
		//float timeUps = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - upsTime).count();
		if (m_keyboardEvent.action == GLFW_REPEAT || m_keyboardEvent.action == GLFW_PRESS)
			m_camera.update(m_keyboardEvent, timeDelta, 1.0f, 90.0f);

		for (size_t i = 0; i < m_meshes.size(); i++) {

			size_t ind = i * m_swapChain.images.size() + currentImage;
			MVPMatrices ubo{};
			glm::mat4 modelTr = glm::mat4(1.0f);
			modelTr[3][0] = 1.0f*i;
			time = 1;

			/*Vulkan: z is up/down. And y is depth*/
			ubo.model = glm::rotate(modelTr, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = m_camera.transformation;				//glm::lookAt(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj = m_camera.projection;					//glm::perspective(glm::radians(45.0f), m_swapChain.extent.width / (float)m_swapChain.extent.height, 0.1f, 10.0f);

			void* data;
			vkMapMemory(m_device.device, m_uniformBuffers[ind].memory, 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(m_device.device, m_uniformBuffers[ind].memory);
		}
		//if (timeUps >= 0.033f) {
		//	upsTime = currentTime;
		//}
		
		lastTime = currentTime;
	}


	/// <summary>
	/// private member function to render.
	/// </summary>
	/// <param name="imageIndex">The image Index currently being processed. (Inflag image)</param>
	void BRDFA_Engine::render(uint32_t imageIndex) {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//imgui commands
		ImGui::ShowDemoWindow();
		ImGui::Render();
		updateUICommandBuffers(m_commander, m_device, m_graphicsPipeline, m_swapChain, imageIndex);


		if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_device.device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}
		m_imagesInFlight[imageIndex] = m_sync[m_currentFrame].f_inFlight;

		VkSemaphore waitSemaphores[] = { m_sync[m_currentFrame].s_imageAvailable };
		VkSemaphore signalSemaphores[] = { m_sync[m_currentFrame].s_renderFinished };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		std::vector<VkCommandBuffer> commands = { m_commander.sceneBuffers[imageIndex], m_commander.uiBuffers[imageIndex] };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = static_cast<uint32_t>(commands.size());
		submitInfo.pCommandBuffers = commands.data();
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

		VkResult result = vkQueuePresentKHR(m_device.presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_frameBufferResized) {
			m_frameBufferResized = false;
			recreate();
		}

		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
	}


	/// <summary>
	/// cleans up the swapchain, command buffers, descriptors, and the graphics pipeline.
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
		vkFreeCommandBuffers(m_device.device, m_commander.pool, static_cast<uint32_t>(m_commander.sceneBuffers.size()), m_commander.sceneBuffers.data());

		/*Clearing the Graphics pipeline*/
		vkDestroyPipeline(m_device.device, m_graphicsPipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(m_device.device, m_graphicsPipeline.layout, nullptr);
		vkDestroyRenderPass(m_device.device, m_graphicsPipeline.sceneRenderPass, nullptr);

		/*Delete the remaining swapchain objects*/
		for (auto imageView : m_swapChain.imageViews) {
			vkDestroyImageView(m_device.device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_device.device, m_swapChain.swapChain, nullptr);

		/*Clearing the Objects related data to recreate them.*/
		/*Deleting the uniform buffers.*/
		for (size_t i = 0; i < m_swapChain.images.size() * m_meshes.size(); i++) {
			vkDestroyBuffer(m_device.device, m_uniformBuffers[i].obj, nullptr);
			vkFreeMemory(m_device.device, m_uniformBuffers[i].memory, nullptr);
		}

		/*Cleaning the skymap image*/
		vkDestroyImageView(m_device.device, m_skymap.view, nullptr);
		vkDestroyImage(m_device.device, m_skymap.obj, nullptr);
		vkFreeMemory(m_device.device, m_skymap.memory, nullptr);
		vkDestroySampler(m_device.device, m_skymap.sampler, nullptr);


		vkDestroyDescriptorPool(m_device.device, m_descriptorData.pool, nullptr);
	}


	/// <summary>
	/// Recreates the whole engine at runtime. Cleans it up first then re-initializes the needed parts.
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
		loadEnvironmentMap(SKYMAP_PATHS);
		createUniformBuffers(m_uniformBuffers, m_commander, m_device, m_swapChain, m_meshes.size());
		initDescriptors(m_descriptorData, m_device, m_swapChain, m_uniformBuffers, m_meshes, m_skymap);
		recordCommandBuffers(m_commander, m_device, m_graphicsPipeline, m_descriptorData, m_swapChain, m_meshes);

		/*Syncronization objects re-initialization.*/
		m_imagesInFlight.resize(m_swapChain.images.size(), VK_NULL_HANDLE);
	}



}