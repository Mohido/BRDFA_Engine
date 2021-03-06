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

#include <imgui_text_editor/TextEditor.h>

// BRDFA_Engine Dependencies
#include "brdfa_engine.hpp"
#include "brdfa_cons.hpp"
#include <helpers/functions.hpp>
#include "brdfa_callbacks.hpp"


#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>



// STD Dependencies
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <thread>
#include <future>



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
	bool BRDFA_Engine::loadObject(const std::string& object_path, const std::vector<std::string>& texture_paths) {
		if (object_path.size() == 0 || texture_paths.size() == 0 || texture_paths[0].size() == 0) {
			std::cout << "INFO: Object and texture paths must be given to load the model. We don't support texture-less models yet." << std::endl;
			return false;
		}
		vkDeviceWaitIdle(m_device.device);
		
		/*Adding the new mesh*/
		m_meshes.push_back(loadMesh(m_commander, m_device, object_path, texture_paths, m_swapChain.images.size()));		// Loading veriaty of objects

		/*Adding a new uniform buffer*/
		size_t oldSize = m_uniformBuffers.size();
		size_t finalSlotsCount = m_swapChain.images.size() * m_meshes.size();
		m_uniformBuffers.resize(finalSlotsCount);	// Adding 1 extra empty slot..
		for (size_t i = oldSize; i < finalSlotsCount; i++) {
			createBuffer(
				m_commander, m_device, VkDeviceSize(sizeof(MVPMatrices)),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
					| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_uniformBuffers[i]);
		}
		
		/*Recreating the Descriptors sets*/
		vkDestroyDescriptorPool(m_device.device, m_descriptorData.pool, nullptr);
		initDescriptors(m_descriptorData, m_device, m_swapChain, m_uniformBuffers, m_meshes, m_skymap);
		
		/*Re-recording the command buffers*/
		vkFreeCommandBuffers(m_device.device, m_commander.pool, static_cast<uint32_t>(m_commander.sceneBuffers.size()), m_commander.sceneBuffers.data());
		recordCommandBuffers(m_commander, m_device, m_graphicsPipelines, m_descriptorData, m_swapChain, m_meshes, m_skymap_mesh, m_skymap_pipeline);
		return true;
	}


	/// <summary>
	/// Called to load that object from the ith object from the scene.
	/// </summary>
	/// <param name="idx"></param>
	/// <returns></returns>
	bool BRDFA_Engine::deleteObject(const int& idx) {
		if (this->m_meshes.size() == 1) {
			std::cout << "You can NOT delete all the meshes!! At least 1 need to be used for the engine to continue running." << std::endl;
			return true;
		}
		vkDeviceWaitIdle(m_device.device);

		/*Deleting the mesh vulkan objects.*/
		destroyMesh(this->m_meshes.at(idx), m_device);
		this->m_meshes.erase(this->m_meshes.begin() + idx);

		/*Adding a new uniform buffer*/
		size_t oldSize = m_uniformBuffers.size();
		size_t finalSlotsCount = m_swapChain.images.size() * m_meshes.size();
		for (size_t i = finalSlotsCount; i < oldSize; i++) {
			vkDestroyBuffer(m_device.device, m_uniformBuffers[i].obj, nullptr);
			vkFreeMemory(m_device.device, m_uniformBuffers[i].memory, nullptr);
		}
		m_uniformBuffers.resize(finalSlotsCount);	// Adding 1 extra empty slot..

		/*Recreating the Descriptors sets*/
		vkDestroyDescriptorPool(m_device.device, m_descriptorData.pool, nullptr);
		initDescriptors(m_descriptorData, m_device, m_swapChain, m_uniformBuffers, m_meshes, m_skymap);

		/*Re-recording the command buffers*/
		vkFreeCommandBuffers(m_device.device, m_commander.pool, static_cast<uint32_t>(m_commander.sceneBuffers.size()), m_commander.sceneBuffers.data());
		recordCommandBuffers(m_commander, m_device, m_graphicsPipelines, m_descriptorData, m_swapChain, m_meshes, m_skymap_mesh, m_skymap_pipeline);
		return true;
	}


	/// <summary>
	/// This function can be called to reload a new skymap to the scene. It can be called only to reload skybox images.
	/// </summary>
	/// <param name="path">Relative path to the skybox image</param>
	/// <returns>If the image was loaoded successfully</returns>
	bool BRDFA_Engine::reloadSkymap(const std::string& path) {
		/*Waiting for the current device to finish what it is doing.*/
		vkDeviceWaitIdle(m_device.device);
		
		/*Meshes dependent*/
		loadEnvironmentMap(path);

		/*Recreating the Descriptors sets*/
		vkDestroyDescriptorPool(m_device.device, m_descriptorData.pool, nullptr);
		initDescriptors(m_descriptorData, m_device, m_swapChain, m_uniformBuffers, m_meshes, m_skymap);

		/*Recording the new skymap mesh */
		vkFreeCommandBuffers(m_device.device, m_commander.pool, static_cast<uint32_t>(m_commander.sceneBuffers.size()), m_commander.sceneBuffers.data());
		recordCommandBuffers(m_commander, m_device, m_graphicsPipelines, m_descriptorData, m_swapChain, m_meshes, m_skymap_mesh, m_skymap_pipeline);
		return true;
	}


	/// <summary>
	/// Loading an environment map that is seperated into 6 different files. Skymap is only supported.
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
	/// 
	/// </summary>
	/// <returns></returns>
	bool BRDFA_Engine::loadEnvironmentMap(const std::string& skyboxSides) {
		/*Loading Image data from file.*/
		int texWidth, texHeight, texChannels;
		stbi_uc* textureData;
		textureData = stbi_load(skyboxSides.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!textureData) {
			throw std::runtime_error("Failed to load image: [" + skyboxSides + "]");
			return false; // we should not break the whole program if we don't find an image...
		}
		
		unsigned int faceWidth = texWidth / 4;
		unsigned int faceHeight = texHeight / 3;

		VkDeviceSize imageSize = faceWidth * faceHeight * 4 * 6;				// Full buffer size (The size of all the images.)
		VkDeviceSize layerSize = faceWidth * faceHeight * 4;			// Size per layer

		m_latest_skymap = skyboxSides;

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
			char* imageData = brdfa::loadFace((char*)textureData, texWidth, texHeight, BoxSide(i));
			memcpy(static_cast<char*>(data) + (layerSize * i), imageData, static_cast<size_t>(layerSize));
			delete[] imageData;
		}
		vkUnmapMemory(m_device.device, staging.memory);

		/*Freeing the Loaded data in the RAM*/
		stbi_image_free(textureData);

		if (m_skymap.width != faceWidth || m_skymap.height != faceHeight) { // If the new image has different resolution
			vkDestroyImageView(m_device.device, m_skymap.view, nullptr);
			vkDestroyImage(m_device.device, m_skymap.obj, nullptr);
			vkFreeMemory(m_device.device, m_skymap.memory, nullptr);

			m_skymap.view = VK_NULL_HANDLE;
			m_skymap.obj = VK_NULL_HANDLE;
			m_skymap.memory = VK_NULL_HANDLE;
		}

		/*Creating an empty buffer in the GPU RAM*/
		if (m_skymap.obj == VK_NULL_HANDLE) {
			createImage(
				m_commander, m_device,
				faceWidth, faceHeight,
				1, VK_SAMPLE_COUNT_1_BIT,
				VK_FORMAT_R8G8B8A8_SRGB,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_skymap, true);
		}

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
			static_cast<uint32_t>(faceWidth), static_cast<uint32_t>(faceHeight));

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

		if (m_skymap.view != VK_NULL_HANDLE) {
			vkDestroyImageView(m_device.device, m_skymap.view, nullptr);
		}

		/*Creating sky map image view for the skymap.*/
		m_skymap.view = createImageView(
			m_skymap.obj, m_device.device,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_skymap.mipLevels, true);

		/*If we are just using the old skymap (It has a sampler, then we don't need to recreate it.)*/
		if (m_skymap.sampler != VK_NULL_HANDLE)
			return true;

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

		this->m_latest_skymap = skyboxSides;
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


	/// <summary>
	/// Registers a Keyboard event to the engine Keyboard event system.
	/// </summary>
	/// <param name="key">key pressed</param>
	/// <param name="action">action</param>
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


	/// <summary>
	/// Registers a mouse event to the engine mouse event system.
	/// </summary>
	/// <param name="button">Mouse Button that has been clicked (1->5)</param>
	/// <param name="action">The mouse action that has been inputted</param>
	void BRDFA_Engine::fireMouseButtonEvent(int button, int action) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			double xpos, ypos;
			glfwGetCursorPos(m_window, &xpos, &ypos);
			m_mouseEvent.init_cords = glm::vec2(float(xpos), float(ypos));
			m_mouseEvent.update = true;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			m_mouseEvent.init_cords = glm::vec2(0.0f, 0.0f);
			m_mouseEvent.delta_cords = glm::vec2(0.0f, 0.0f);
			m_mouseEvent.update = false;
		}

	}


// ------------------------------------------------ MEMBER FUNCTIONS ---------------------------------------

	/// <summary>
	/// This is called to refresh a specific object. Refreshing means re-recording.
	/// </summary>
	/// <param name="idx"></param>
	void BRDFA_Engine::refreshObject(const size_t& idx) {
		/*Waiting for the device to be free.*/
		vkDeviceWaitIdle(m_device.device);

		/*Re-recording the command buffers*/
		vkFreeCommandBuffers(m_device.device, m_commander.pool, static_cast<uint32_t>(m_commander.sceneBuffers.size()), m_commander.sceneBuffers.data());
		recordCommandBuffers(m_commander, m_device, m_graphicsPipelines, m_descriptorData, m_swapChain, m_meshes, m_skymap_mesh, m_skymap_pipeline);
	}


	/// <summary>
	/// 
	/// </summary>
	/// <param name="brdfName"></param>
	/// <param name="cacheIt"></param>
	void BRDFA_Engine::saveBRDF(const std::string& brdfName, const bool& cacheIt)
	{
		/*Saving the BRDF fragment file*/
		std::string textPath = "shaders/brdfs/";
		std::ofstream fileBRDF_text( (textPath + brdfName + std::string(".brdf")).c_str(), std::ofstream::out);
		if (!fileBRDF_text) {
			std::cout << "ERROR: CAN'T OPEN FILE TO SAVE BRDF" << std::endl;
			return;
		}
		fileBRDF_text << m_loadedBrdfs.at(brdfName).glslPanel.GetText();
		fileBRDF_text.close();

		/*Cache it if needed*/
		std::string cachePath = "shaders/cache/";
		std::ofstream fileBRDF_spir;
		if (cacheIt) {
			fileBRDF_spir.open((cachePath + brdfName + std::string(".spv")), std::ofstream::out | std::ofstream::binary);
			if (!fileBRDF_spir) {
				std::cout << "ERROR: CAN'T CACHE BRDF" << std::endl;
				return;
			}
			for (size_t i = 0; i < m_loadedBrdfs.at(brdfName).latest_spir_v.size(); i++) {
				fileBRDF_spir << m_loadedBrdfs.at(brdfName).latest_spir_v.at(i);
			}
			fileBRDF_spir.close();
		}
		std::cout << "BRDFs have been saved." << std::endl;
	}


	/// <summary>
	/// 
	/// </summary>
	/// <param name="brdfName"></param>
	/// <param name="fragSpirv"></param>
	void BRDFA_Engine::recreatePipeline(const std::string& brdfName, const std::vector<char>& fragSpirv, const bool& refreshObj)
	{
		/*Destroying old pipeline*/
		vkDeviceWaitIdle(m_device.device);
		if (m_graphicsPipelines.pipelines.find(brdfName) != m_graphicsPipelines.pipelines.end())	// if found then destroy the old pipeline
			vkDestroyPipeline(m_device.device, m_graphicsPipelines.pipelines.at(brdfName), nullptr);
		else   // if not found then create a new one.
			m_graphicsPipelines.pipelines.insert({ brdfName , VK_NULL_HANDLE });

		/*Creating a new pipeline*/
		createGraphicsPipeline(
			m_graphicsPipelines.layout, m_graphicsPipelines.sceneRenderPass,
			m_graphicsPipelines.pipelines.at(brdfName), m_skymap_pipeline,
			m_device, m_swapChain, m_descriptorData, m_vertSpirv, fragSpirv, false);

		/*Re record the scene objects*/
		for (size_t j = 0; j < m_meshes.size() & refreshObj; j++)
			refreshObject(j);
	}


	/// <summary>
	/// 
	/// </summary>
	/// <param name="brdfName"></param>
	/// <param name="fragSpirv"></param>
	void BRDFA_Engine::addPipeline(const std::string& brdfName, const std::vector<char>& fragSpirv)
	{
		/*Destroying old pipeline*/
		vkDeviceWaitIdle(m_device.device);
		if (m_graphicsPipelines.pipelines.find(brdfName) != m_graphicsPipelines.pipelines.end()) {
			throw std::runtime_error("ERROR: BRDF Pipeline can't be added. It already exists!.");
		}

		m_graphicsPipelines.pipelines.insert({ brdfName , VK_NULL_HANDLE });

		/*Creating a new pipeline*/
		createGraphicsPipeline(
			m_graphicsPipelines.layout, m_graphicsPipelines.sceneRenderPass,
			m_graphicsPipelines.pipelines.at(brdfName), m_skymap_pipeline,
			m_device, m_swapChain, m_descriptorData, m_vertSpirv, fragSpirv, false);

		/*Re record the scene objects*/
		for (size_t j = 0; j < m_meshes.size(); j++) refreshObject(j);
	}


	/// <summary>
	/// Load all the needed pipelines. 
	///		* We load the cached BRDFs first. Cached means pre-compiled BRDFs.
	///		* We load all the source codes of the BRDFs second.
	///		* We create graphics pipelines from the cached spir-v shaders.
	///		* Configurations can be: 
	///				1) Hot-Start: which only uses the cache without compiling and building the rest of the BRDFs.		
	///				2) Full Load: Which will load all the BRDFs that are not cached, compile them, cache them and then build the graphics pipelines.  
	///					This method will take a bit longer because of the compilation step for the non-cached brdfs.
	///								
	/// The whole BRDF source codes will be loaded to the engine.
	/// 
	/// </summary>
	void BRDFA_Engine::loadPipelines() {
		/*Craetion of a pipeline layout*/
		createPipelineLayout(m_graphicsPipelines, m_device, m_descriptorData);

		/*Creation of objects Pipelines*/
		std::string mainShader_f = (SHADERS_PATH + "/main.frag");
		std::string mainShader_v = (SHADERS_PATH + "/main.vert");
		std::string brdfs = (SHADERS_PATH + "/brdfs");
		std::string cache = (SHADERS_PATH + "/cache");

		/*Loading the basic.spv (basic rendering.)*/
		m_vertSpirv = readFile(SHADERS_PATH + "/vert.spv", true);
		auto frag_main_shader_code = readFile(SHADERS_PATH + "/basic.spv", true);
		m_graphicsPipelines.pipelines.insert({ "None" , {} });
		createGraphicsPipeline(
			m_graphicsPipelines.layout, m_graphicsPipelines.sceneRenderPass,
			m_graphicsPipelines.pipelines.at("None"), m_skymap_pipeline,
			m_device, m_swapChain, m_descriptorData, m_vertSpirv, frag_main_shader_code, false);

		/*Loading the extra BRDFs*/
		frag_main_shader_code.clear();
		frag_main_shader_code = readFile(SHADERS_PATH + "/main.frag", false);
		m_mainFragShader = std::string(frag_main_shader_code.begin(), frag_main_shader_code.end());

		for (const auto& entry : std::filesystem::directory_iterator(brdfs)) {
			std::string temp = entry.path().string();
			std::size_t found = temp.find_last_of("/\\");		// Finding splitters
			std::string brdfFilePath = temp.substr(0, found);
			std::string brdfFileName = temp.substr(found+1);

			printf("[INFO]: Loading file: %s\n", temp.c_str());
			int extInd = brdfFileName.find(".brdf");
			bool isbrdf = extInd != std::string::npos;
			if (isbrdf) {
				/*BRDF name as a key and the shader file path.*/
				std::string brdfName(brdfFileName.begin() , brdfFileName.begin() + extInd);
				std::string shaderPath = brdfFilePath + "/" + brdfFileName;
				printf("[INFO]: Loading BRDF: %s\n", brdfFileName.c_str());

				/*Check if current file exist in cache. And if it exists, load it from there.*/
				bool loadCache = false;
				for (const auto& entry_c : std::filesystem::directory_iterator(cache)) {
					if (m_configuration.no_cache_load) break;
					temp = entry_c.path().string();
					found = temp.find_last_of("/\\");		// Finding splitters
					std::string cacheFilePath = temp.substr(0, found);
					std::string cacheFileName = temp.substr(found + 1);
					extInd = cacheFileName.find(".spv");
					std::string cacheName(cacheFileName.begin(), cacheFileName.begin() + extInd);
					if (cacheName == brdfName) {
						loadCache = true;
						printf("[INFO]: Loading \"%s\" BRDF from its Cache\n", brdfName.c_str());
						break;
					}
				}

				/*Check if the pipeline with the name exists.*/
				if (m_graphicsPipelines.pipelines.find(brdfName) != m_graphicsPipelines.pipelines.end()) {
					printf("[WARNING]: BRDF Pipeline already created.\n");
					continue;
				}

				/*Form the whole fragment shader*/
				auto brdf_shader_code = readFile(shaderPath, false);
				std::string brdf_s(brdf_shader_code.begin(), brdf_shader_code.end());
				//std::string mc(frag_main_shader_code.begin(), frag_main_shader_code.end());
				
				/*Concatenating the source code with filtering the terminations*/
				std::string concat;
				concat.reserve(brdf_s.length() + m_mainFragShader.length());
				for (int i = 0; i < m_mainFragShader.size(); i++)			
					concat = (m_mainFragShader[i] == '\0') ? concat : concat + m_mainFragShader[i];
				for (int i = 0; i < brdf_s.size(); i++)		
					concat = (brdf_s[i] == '\0') ? concat : concat + brdf_s[i];

				/*If we reach this point, it means that we will insert the loaded brdf into our loaded brdfs panel*/
				
				if (m_loadedBrdfs.find(brdfName) == m_loadedBrdfs.end()) {
					BRDF_Panel	lp;
					lp.brdfName = brdfName;
					lp.glslPanel.SetText(brdf_s);
					if (m_configuration.hot_load) {
						lp.tested = false;
						m_loadedBrdfs.insert({ brdfName, lp });
						continue;
					}
					std::string cacheFileName = cache + "/" + brdfName + ".spv";
					/*Insert a new pipeline.*/
					m_graphicsPipelines.pipelines.insert({ brdfName , {} });
					if (loadCache) {
						compilationPool.push_back(std::thread(threadAddSpirv, cacheFileName, lp, &m_loadedBrdfs));
					}
					else {
						compilationPool.push_back(std::thread(threadCompileGLSL, concat, lp, &m_loadedBrdfs, false));
					}
				}
			}
		}
			
		for (auto & t : compilationPool) {
			t.join();
		}
		compilationPool.clear();

		for (const auto& it : m_loadedBrdfs) {
			createGraphicsPipeline(
				m_graphicsPipelines.layout, m_graphicsPipelines.sceneRenderPass,
				m_graphicsPipelines.pipelines.at(it.second.brdfName), m_skymap_pipeline,
				m_device, m_swapChain, m_descriptorData, m_vertSpirv, it.second.latest_spir_v, false);
		}
		


		/*Creation of skymap pipelines*/
		auto vert_sky_shader_code = readFile(SHADERS_PATH + "/skybox.vert.spv", true);
		auto frag_sky_shader_code = readFile(SHADERS_PATH + "/skybox.frag.spv", true);
		createGraphicsPipeline(
			m_graphicsPipelines.layout, m_graphicsPipelines.sceneRenderPass,
			m_graphicsPipelines.pipelines.begin()->second, m_skymap_pipeline,
			m_device, m_swapChain, m_descriptorData, 
			vert_sky_shader_code, frag_sky_shader_code, true);
	}


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
		glfwSetMouseButtonCallback(m_window, mouseButtonCallback);

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
		createRenderPass(m_graphicsPipelines, m_device, m_swapChain);
		createDescriptorSetLayout(m_descriptorData, m_device, m_swapChain);
		// auto vertShaderCode = readFile("shaders/main.vert", false);
		// auto fragShaderCode = readFile("shaders/main.frag", false);
		// auto spirVShaderCode_vert = compileShader(vertShaderCode, true, "vertexShader");
		// auto spirVShaderCode_frag = compileShader(fragShaderCode, false, "FragmentSHader");
		// createGraphicsPipeline(m_graphicsPipeline, m_skymap_pipeline, m_device, m_swapChain, m_descriptorData, spirVShaderCode_vert, spirVShaderCode_frag);
		this->loadPipelines();
		createCommandPool(m_commander.pool, m_device);
		createFramebuffers(m_swapChain, m_commander, m_device, m_graphicsPipelines);
		createSyncObjects(m_sync, m_imagesInFlight, m_device, m_swapChain, MAX_FRAMES_IN_FLIGHT);

		/*SCENE Initalization. Related functionalities.*/
		m_meshes.push_back(loadMesh(m_commander, m_device, MODEL_PATH, TEXTURE_PATH, m_swapChain.images.size() ));		// Loading veriaty of objects
		loadVertices(m_skymap_mesh, m_commander, m_device, CUBE_MODEL_PATH);				// Loading skymap vertices (CUBE)
		loadEnvironmentMap(SKYMAP_PATHS);
		m_camera = Camera(m_swapChain.extent.width, m_swapChain.extent.height, 0.1f, 100.0f, 45.0f);

		createUniformBuffers(m_uniformBuffers, m_commander, m_device, m_swapChain, m_meshes.size());
		initDescriptors(m_descriptorData, m_device, m_swapChain, m_uniformBuffers, m_meshes, m_skymap);
		
		/*Recording the command buffers.*/
		recordCommandBuffers(m_commander, m_device, m_graphicsPipelines, m_descriptorData, m_swapChain, m_meshes, m_skymap_mesh, m_skymap_pipeline);

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

		bool initV = ImGui_ImplVulkan_Init(&init_info, m_graphicsPipelines.sceneRenderPass);

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
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();


		/*Update the camera iff none of the imgui windows are focused.*/
		if (!m_uistate.focused) { // update camera
			double xpos, ypos;
			glfwGetCursorPos(m_window, &xpos, &ypos);

			glm::vec2 temp = glm::vec2(float(xpos), float(ypos)) - m_mouseEvent.init_cords;
			bool updateKeysOnly = (temp.x - m_mouseEvent.delta_cords.x == 0.0f && temp.y - m_mouseEvent.delta_cords.y == 0.0f);

			if (updateKeysOnly) {
				MouseEvent dull = m_mouseEvent;
				dull.update = false;
				m_camera.update(m_keyboardEvent, dull, timeDelta);
			}
			else {
				m_mouseEvent.delta_cords = temp;
				m_camera.update(m_keyboardEvent, m_mouseEvent, timeDelta);
			}
		} // end update camera system
		

		for (size_t i = 0; i < m_meshes.size(); i++) { // setup ubos for meshes
			size_t ind = i * m_swapChain.images.size() + currentImage;
			MVPMatrices ubo{};
			ubo.model = m_meshes[i].getFinalTransformation();			//glm::rotate(modelTr, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = m_camera.transformation;				//glm::lookAt(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj = m_camera.projection;					//glm::perspective(glm::radians(45.0f), m_swapChain.extent.width / (float)m_swapChain.extent.height, 0.1f, 10.0f);
			ubo.pos_c = m_camera.position;
			ubo.render_opt = glm::vec3(m_meshes[i].extra[0], m_meshes[i].extra[1], static_cast<float>(m_meshes[i].samples));

			void* data;
			vkMapMemory(m_device.device, m_uniformBuffers[ind].memory, 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(m_device.device, m_uniformBuffers[ind].memory);

			vkMapMemory(m_device.device, m_meshes[i].paramsBuffer[currentImage].memory, 0, sizeof(m_meshes[i].params), 0, &data);
			memcpy(data, &m_meshes[i].params, sizeof(m_meshes[i].params));
			vkUnmapMemory(m_device.device, m_meshes[i].paramsBuffer[currentImage].memory);
		}// end setup ubos 
		
		lastTime = currentTime;
	}


	/// <summary>
	/// private member function to render.
	/// </summary>
	/// <param name="imageIndex">The image Index currently being processed. (Inflag image)</param>
	void BRDFA_Engine::render(uint32_t imageIndex) {
		auto startTime = std::chrono::high_resolution_clock::now();
		this->drawUI(imageIndex);
		
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

		if (this->saveShot)
			record(m_currentFrame);

		auto endtime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(endtime - startTime).count();
		m_uistate.timePerFrame = (m_uistate.timePerFrame > 0.0f)? (m_uistate.timePerFrame + time * 1000.0f) / 2.0f : time * 1000.0f;
	}

	/// <summary>
	///		Saves the current frame into a file. The file will be the saveFrameDir memeber variable set in the engine.
	/// </summary>
	/// <param name="imageIndex"></param>
	void BRDFA_Engine::record(uint32_t imageIndex)
	{
		vkDeviceWaitIdle(m_device.device);
		bool supportsBlit = true;

		// Check blit support for source and destination
		VkFormatProperties formatProps;

		// Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
		vkGetPhysicalDeviceFormatProperties(m_device.physicalDevice, m_swapChain.format, &formatProps);
		if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
			std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
			supportsBlit = false;
		}

		// Check if the device supports blitting to linear images
		vkGetPhysicalDeviceFormatProperties(m_device.physicalDevice, m_swapChain.format, &formatProps);
		if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
			std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
			supportsBlit = false;
		}

		// Source for the copy is the last rendered swapchain image
		Image srcImage;
		srcImage.obj = m_swapChain.images[imageIndex];
		srcImage.width = m_swapChain.extent.width;
		srcImage.height = m_swapChain.extent.height;
		srcImage.mipLevels = 1;

		// Create the image
		Image dstImage;
		createImage(
			m_commander, m_device, m_swapChain.extent.width, m_swapChain.extent.height, 1, VK_SAMPLE_COUNT_1_BIT,
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, dstImage);

		// Do the actual blit from the swapchain image to our host visible destination image
		// Transition destination image to transfer destination layout
		transitionImageLayout(dstImage, m_commander, m_device, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		transitionImageLayout(srcImage, m_commander, m_device, m_swapChain.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		// If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
		if (supportsBlit)
		{
			// Define the region to blit (we will blit the whole swapchain image)
			VkOffset3D blitSize;
			blitSize.x = m_swapChain.extent.width;
			blitSize.y = m_swapChain.extent.height;
			blitSize.z = 1;
			VkImageBlit imageBlitRegion{};
			imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.srcSubresource.layerCount = 1;
			imageBlitRegion.srcOffsets[1] = blitSize;
			imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.dstSubresource.layerCount = 1;
			imageBlitRegion.dstOffsets[1] = blitSize;

			VkCommandBuffer copyCmd = beginSingleTimeCommands(m_commander, m_device);
			// Issue the blit command
			vkCmdBlitImage(
				copyCmd,
				srcImage.obj, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage.obj, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlitRegion,
				VK_FILTER_NEAREST);
			endSingleTimeCommands(m_commander, m_device);
		}
		else
		{
			// Otherwise use image copy (requires us to manually flip components)
			VkImageCopy imageCopyRegion{};
			imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.srcSubresource.layerCount = 1;
			imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.dstSubresource.layerCount = 1;
			imageCopyRegion.extent.width = m_swapChain.extent.width;
			imageCopyRegion.extent.height = m_swapChain.extent.height;
			imageCopyRegion.extent.depth = 1;
			VkCommandBuffer copyCmd = beginSingleTimeCommands(m_commander, m_device);
			// Issue the copy command
			vkCmdCopyImage(
				copyCmd,
				srcImage.obj, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage.obj, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageCopyRegion);
			endSingleTimeCommands(m_commander, m_device);
		}

		// Transition destination image to general layout, which is the required layout for mapping the image memory later on
		transitionImageLayout(srcImage, m_commander, m_device, m_swapChain.format, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		transitionImageLayout(dstImage, m_commander, m_device, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

		// Get layout of the image (including row pitch)
		VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
		VkSubresourceLayout subResourceLayout;
		vkGetImageSubresourceLayout(m_device.device, dstImage.obj, &subResource, &subResourceLayout);


		/*Save the image into a raw file*/
		{
			const char* data;
			vkMapMemory(m_device.device, dstImage.memory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
			data += subResourceLayout.offset;
	
			// If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
			bool colorSwizzle = false;

			// Check if source is BGR
			// Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
			if (!supportsBlit)
			{
				std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
				colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), m_swapChain.format) != formatsBGR.end());
			}


			std::vector<char> rgbPixels;
			rgbPixels.reserve(m_swapChain.extent.height * m_swapChain.extent.width * 3);

			for (uint32_t y = 0; y < m_swapChain.extent.height; y++)
			{
				unsigned int* row = (unsigned int*)data;
				for (uint32_t x = 0; x < m_swapChain.extent.width; x++)
				{
					if (colorSwizzle)
					{
						unsigned char* cp = (unsigned char*)row;
						int ip[3] = {
							int((unsigned char)cp[2]),
							int((unsigned char)cp[1]),
							int((unsigned char)cp[0]) };
						rgbPixels.push_back(static_cast<char>(ip[0]));
						rgbPixels.push_back(static_cast<char>(ip[1]));
						rgbPixels.push_back(static_cast<char>(ip[2]));
					}
					else
					{
						//ofs.write((char*)row, 3);
						unsigned char* cp = (unsigned char*)row;
						int ip[3] = { 
							int((unsigned char)cp[0]), 
							int((unsigned char)cp[1]), 
							int((unsigned char)cp[2])};

						rgbPixels.push_back(static_cast<char>(ip[0]));
						rgbPixels.push_back(static_cast<char>(ip[1]));
						rgbPixels.push_back(static_cast<char>(ip[2]));
					}
					row++;
				}
				// ss << "\n";
				data += subResourceLayout.rowPitch;
			}

			std::string fullSaveName = (savedFramesDir + std::string("_stbi.bmp")).c_str();
			stbi_write_bmp(fullSaveName.c_str(), m_swapChain.extent.width, m_swapChain.extent.height, 3, rgbPixels.data());

			//rgbf.close();
			printf("[INFO]: [record]: Image saved at the path: %s\n", fullSaveName.c_str());
		
			// Clean up resources
			vkUnmapMemory(m_device.device, dstImage.memory);
			vkFreeMemory(m_device.device, dstImage.memory, nullptr);
			vkDestroyImage(m_device.device, dstImage.obj, nullptr);
			this->saveShot = false;
		}
	}


	/// <summary>
	/// 
	/// </summary>
	/// <param name="imageIndex"></param>
	void BRDFA_Engine::drawUI(uint32_t imageIndex) {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		/*Create the engine menu window*/
		this->drawUI_menubar();
		this->drawUI_objectLoader();
		this->drawUI_skymapLoader();
		this->drawUI_logger();
		this->drawUI_objects();
		this->drawUI_camera();
		this->drawUI_tester();
		this->drawUI_editorBRDF();
		this->drawUI_comparer();
		this->drawUI_frameSaver(imageIndex);
		

		// {
		// 	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		// 	if (ImGui::BeginTabBar("WindowTabs", tab_bar_flags))
		// 	{
		// 		if (ImGui::BeginTabItem("Objects"))
		// 		{
		// 			drawUI_objects();
		// 			ImGui::EndTabItem();
		// 		}
		// 		if (ImGui::BeginTabItem("Camera"))
		// 		{
		// 			drawUI_camera();
		// 			ImGui::EndTabItem();
		// 		}
		// 		if (ImGui::BeginTabItem("Advance"))
		// 		{
		// 			drawUI_editorBRDF();
		// 			ImGui::EndTabItem();
		// 		}
		// 		ImGui::EndTabBar();
		// 	}
		// 	ImGui::Separator();
		// }

		
		ImGui::Render();

		// Update the m_uistate and checks if any of the imgui windows are focused.
		m_uistate.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
		updateUICommandBuffers(m_commander, m_device, m_graphicsPipelines, m_swapChain, imageIndex);
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
		m_commander.sceneBuffers.clear();

		/*Clearing the Graphics pipeline*/
		for (auto& it : m_graphicsPipelines.pipelines) {
			vkDestroyPipeline(m_device.device, it.second , nullptr);
		}
		m_graphicsPipelines.pipelines.clear();

		vkDestroyPipeline(m_device.device, m_skymap_pipeline, nullptr);
		
		vkDestroyPipelineLayout(m_device.device, m_graphicsPipelines.layout, nullptr);
		vkDestroyRenderPass(m_device.device, m_graphicsPipelines.sceneRenderPass, nullptr);

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
		createRenderPass(m_graphicsPipelines, m_device, m_swapChain);
		createDescriptorSetLayout(m_descriptorData, m_device, m_swapChain);
		// auto vertShaderCode = readFile("shaders/main.vert", false);
		// auto fragShaderCode = readFile("shaders/main.frag", false);
		// auto spirVShaderCode_vert = compileShader(vertShaderCode, true, "vertexShader");
		// auto spirVShaderCode_frag = compileShader(fragShaderCode, false, "FragmentSHader");
		// createGraphicsPipeline(m_graphicsPipeline, m_skymap_pipeline, m_device, m_swapChain, m_descriptorData, spirVShaderCode_vert, spirVShaderCode_frag);
		// loadPipelines();
		createPipelineLayout(m_graphicsPipelines, m_device, m_descriptorData);
		createFramebuffers(m_swapChain, m_commander, m_device, m_graphicsPipelines);


		/*Meshes dependent*/
		this->loadEnvironmentMap(SKYMAP_PATHS);

		createUniformBuffers(m_uniformBuffers, m_commander, m_device, m_swapChain, m_meshes.size());
		initDescriptors(m_descriptorData, m_device, m_swapChain, m_uniformBuffers, m_meshes, m_skymap);

		/*Loading the main pipeline*/
		m_vertSpirv = readFile(SHADERS_PATH + "/vert.spv", true);
		auto frag_main_shader_code = readFile(SHADERS_PATH + "/basic.spv", true);
		m_graphicsPipelines.pipelines.insert({ "None" , {} });
		createGraphicsPipeline(
			m_graphicsPipelines.layout, m_graphicsPipelines.sceneRenderPass,
			m_graphicsPipelines.pipelines.at("None"), m_skymap_pipeline,
			m_device, m_swapChain, m_descriptorData, m_vertSpirv, frag_main_shader_code, false);

		/*Reloading the skymap pipeline*/
		auto vert_sky_shader_code = readFile(SHADERS_PATH + "/skybox.vert.spv", true);
		auto frag_sky_shader_code = readFile(SHADERS_PATH + "/skybox.frag.spv", true);
		createGraphicsPipeline(
			m_graphicsPipelines.layout, m_graphicsPipelines.sceneRenderPass,
			m_graphicsPipelines.pipelines.begin()->second, m_skymap_pipeline,
			m_device, m_swapChain, m_descriptorData,
			vert_sky_shader_code, frag_sky_shader_code, true);

		for (const auto& brdf : m_loadedBrdfs) {
			recreatePipeline(brdf.first, brdf.second.latest_spir_v, false);
		}
		for (const auto& brdf : m_costumBrdfs) {
			recreatePipeline(brdf.first, brdf.second.latest_spir_v, false);
		}

		recordCommandBuffers(m_commander, m_device, m_graphicsPipelines, m_descriptorData, m_swapChain, m_meshes, m_skymap_mesh, m_skymap_pipeline);



		/*Syncronization objects re-initialization.*/
		m_imagesInFlight.resize(m_swapChain.images.size(), VK_NULL_HANDLE);

		if (this->m_latest_skymap.size() > 0)
			this->reloadSkymap(this->m_latest_skymap);
	}



	/// <summary>
	/// Draws the Objects Panel. The user can change the objects parameters here.
	/// </summary>
	void BRDFA_Engine::drawUI_objects() {
		if (!m_uistate.objWindowActive)
			return;


		ImGui::SetNextWindowSize(ImVec2(this->m_configuration.width / 3, 0), ImGuiCond_Appearing);
		ImGui::Begin("Object Viewer", &m_uistate.objWindowActive);

		//ImGuiStyle& style = ImGui::GetStyle();
		//float spacing = style.ItemInnerSpacing.x;
		float button_sz = ImGui::GetFrameHeight();

		// Rendering Meshes data. (Per mesh.)
		for (int i = 0; i < m_meshes.size(); i++) {
			std::string curObj = "Object_" + std::to_string(i + 1);
			const char* current_item = m_meshes[i].renderOption.c_str(); //m_uistate.optionLabels[m_meshes[i].renderOption];

			// Starting the section of the object
			//
			ImGui::BeginChild(curObj.data(), ImVec2(0.0f, button_sz * (12.0f + float(m_meshes[i].shownParameters)*1.2f)), false);

			{// Tab menu of the object
				ImGui::BeginTabBar(curObj.data());
				ImGui::BeginTabItem(curObj.data());
				ImGui::EndTabItem();
				ImGui::EndTabBar();
			} // Tab menu of the object
			{ // Rendering the Rendering options.
				//float w = ImGui::CalcItemWidth();
				//ImGui::PushItemWidth(w - spacing * 5.0f - button_sz * 2.0f);
				if (ImGui::BeginCombo("Rendering Obtions", current_item)) // The second parameter is the label previewed before opening the combo.
				{
					bool refreshEngine = false;
					for (auto& it : m_graphicsPipelines.pipelines)
					{
						bool is_selected = strcmp(current_item, it.first.c_str()); // You can store your selection however you want, outside or inside your objects
						if (ImGui::Selectable(it.first.c_str(), is_selected)) {
							if (is_selected) {
								printf("[INFO]: Has been selected: %s\n", it.first.c_str());
								m_meshes[i].renderOption = it.first;
								ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
								refreshEngine = true;
							}
						}
					}
					if (refreshEngine)
						refreshObject(i);
					ImGui::EndCombo();
				}
			} // Rendering_options rendered
			ImGui::Separator();
			{ // Object Translation option
				float trans[3] = { m_meshes[i].translation[0] , m_meshes[i].translation[1], m_meshes[i].translation[2] };
				ImGui::DragFloat3("Translation", trans, 0.01f);
				m_meshes[i].translation[0] = trans[0];
				m_meshes[i].translation[1] = trans[1];
				m_meshes[i].translation[2] = trans[2];
			} // Object Translation option
			{ // Object Scale option
				float trans[3] = { m_meshes[i].scale[0] , m_meshes[i].scale[1], m_meshes[i].scale[2] };
				ImGui::DragFloat3("Scaler", trans, 0.01f);
				m_meshes[i].scale[0] = trans[0];
				m_meshes[i].scale[1] = trans[1];
				m_meshes[i].scale[2] = trans[2];
			} // Object scale option
			{ // Object Scale option
				float trans[3] = { m_meshes[i].rotation[0] , m_meshes[i].rotation[1], m_meshes[i].rotation[2] };
				ImGui::DragFloat3("Rotation", trans, 0.05f);
				m_meshes[i].rotation = glm::vec3(trans[0], trans[1], trans[2]);
			} // Object scale option
			ImGui::Separator();
			{ // Object extra parameters
				float prms[9] = {
						m_meshes[i].params.extra012.x, m_meshes[i].params.extra012.y, m_meshes[i].params.extra012.z,
						m_meshes[i].params.extra345.x, m_meshes[i].params.extra345.y, m_meshes[i].params.extra345.z,
						m_meshes[i].params.extra678.x, m_meshes[i].params.extra678.y, m_meshes[i].params.extra678.z
				};

				float iw = ImGui::CalcItemWidth();
				ImGui::PushItemWidth(iw * 0.5);
				/*Draw the extra parameters inputs*/
				for (int j = 0; j < m_meshes[i].shownParameters; j++) {
					std::string label = std::string("iParameter") + std::to_string(j);
					ImGui::DragFloat(label.c_str(), &prms[j], 0.001f, 0.0f, 1.0f, "%.4f", 1.0f);
				}

				/*Add/delete parameters buttons*/
				if (m_meshes[i].shownParameters > 0) {
					if (ImGui::Button("-", ImVec2(50, 0))) {
						prms[m_meshes[i].shownParameters - 1] = 0.0;
						m_meshes[i].shownParameters--;
					}
				}
				if (m_meshes[i].shownParameters > 0 && m_meshes[i].shownParameters < 9) 
					ImGui::SameLine();
				if (m_meshes[i].shownParameters < 9) {
					if (ImGui::Button("+", ImVec2(50, 0)))
						m_meshes[i].shownParameters++;
				}
				ImGui::SameLine();
				ImGui::TextWrapped("Add/Delete Parameters Stack");

				/* Copy back the added paramters*/
				m_meshes[i].params.extra012.x = prms[0];	m_meshes[i].params.extra012.y = prms[1];	m_meshes[i].params.extra012.z = prms[2];
				m_meshes[i].params.extra345.x = prms[3];	m_meshes[i].params.extra345.y = prms[4];	m_meshes[i].params.extra345.z = prms[5];
				m_meshes[i].params.extra678.x = prms[6];	m_meshes[i].params.extra678.y = prms[7];	m_meshes[i].params.extra678.z = prms[8];
			} // Object extra parameters
			ImGui::Separator();
			{
				ImGui::InputInt("Light Samples", &m_meshes[i].samples, 1, 10);
				ImGui::PopItemWidth();
			}
			{ // Object deletion button
				ImGui::NewLine();
				if (ImGui::Button("Delete")) {
					this->deleteObject(i);
					std::cout << "Deleting: " << curObj << std::endl;
				}
			} // Object deletion button

			ImGui::EndChild();
		} // For loop over objects

		ImGui::End();
	}


	/// <summary>
	/// Drarws the Camera panel. The user can edit the camera parameters such as movement speed_t, zoom, rotate, 
	/// and change the projection matrix
	/// </summary>
	void BRDFA_Engine::drawUI_camera() {
		if (!m_uistate.camWindowActive)
			return;

		ImGui::SetNextWindowSize(ImVec2(this->m_configuration.width / 3, 0), ImGuiCond_Appearing);
		ImGui::Begin("Camera Editor", &m_uistate.camWindowActive, ImGuiWindowFlags_NoResize);

		{// Tab menu just to look cool
			ImGui::BeginTabBar("");
			ImGui::BeginTabItem("Camera");
			ImGui::EndTabItem();
			ImGui::EndTabBar();
		} // Tab menu of the object
		{ // Position
			float trans[3] = { m_camera.position[0] , m_camera.position[1], m_camera.position[2] };
			ImGui::DragFloat3("Position", trans, 0.01f);
			m_camera.position = glm::vec3(trans[0], trans[1], trans[2]);
			m_camera.updateViewMatrix();
		} // Position
		ImGui::Separator();
		{// Rotation
			float trans[2] = { m_camera.rotation[0] , m_camera.rotation[1] };
			ImGui::DragFloat2("Rotation", trans, 1.0f);
			trans[1] = (trans[1] > 89.0f) ? 89.0f : trans[1];
			trans[1] = (trans[1] < -89.0f) ? -89.0f : trans[1];
			m_camera.rotation = glm::vec3(trans[0], trans[1], trans[2]);
			m_camera.updateDirection();
			m_camera.updateViewMatrix();
		}// Rotation
		ImGui::Separator();
		{// Speed
			ImGui::DragFloat("Movement Speed", &m_camera.speed_t, 0.01f, 0.05f, 5.0f);
			ImGui::DragFloat("Rotation Speed", &m_camera.speed_r, 0.01f, 0.05f, 5.0f);
		}// Speed
		ImGui::End();
	}


	/// <summary>
	///		Draws the Advance panel. In advance panel, the user can insert new GLSL BRDFs, Test/Compile/Edit/Save them. 
	///		
	/// </summary>
	void BRDFA_Engine::drawUI_editorBRDF() {
		if (!m_uistate.brdfEditorWindowActive)
			return;

		
		//ImGuiWindowFlags file_reader_flags = ImGuiWindowFlags_NoMove;// ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
		ImGui::SetNextWindowPos(ImVec2(this->m_configuration.width - this->m_configuration.width / 3, 0), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(this->m_configuration.width / 3, this->m_configuration.height), ImGuiCond_Appearing);
		ImGui::Begin("BRDF Editor", &m_uistate.brdfEditorWindowActive);
		/*Draw Loaded BRDFs*/
		


		if (ImGui::CollapsingHeader("Help")) {
			ImGui::SetWindowFontScale(1.2);
			if (ImGui::TreeNode("Editting BRDFs")) {
				ImGui::TextWrapped("Here you can create a new BRDF, or edit the pre-existing saved ones.");
				ImGui::TextWrapped("\t* To create a new BRDF please click on the costum BRDF drop down.Then insert a new BRDF name and click the + button.After that, a temporary BRDF template will be created.The temporary BRDF will not be saved unless it is tested, then can be loaded to the main BRDFs. ");
				ImGui::TextWrapped("\t* To edit an existed BRDF, you can open the Loaded BRDFs panel and edit the BRDF that you are interested in changing. The changes will not occur, unless you test them first. After testing them, you can view them (Update the Rendering Options). You can save the BRDF to a file and cache it with the save button. The files can be found in the \"./shaders/brdfs\"");
				ImGui::TreePop();
			}// end Editting BRDFs panel
			if (ImGui::TreeNode("Shader Globals")) {
				ImGui::TextWrapped("We have some global uniform variables that you can use in the BRDF code. The variables are the following:");
				ImGui::TextWrapped("* iTexture0, iTexture1, iTexture2, iTexture3");
				ImGui::TextWrapped("\tThese are texture parameters that holds the values of the textures filled when the mesh is loaded. iTexture0 must be loaded within the mesh, while the other textures are optional. However, using a texture that is not being filled will cause an error. Therefore, make sure to use textures if you have added them.");
				ImGui::TextWrapped("* iParameter0, iParameter1, iParameter2, iParameter3");
				ImGui::TextWrapped("\tThese are parameters that can be edited from the object_n interface. We have 4 available parameters that you can pass to the shader and use them. All of the parameters are normalized (0 to 1) floating numbers.");
				ImGui::TreePop();
			}
			ImGui::SetWindowFontScale(1.0);
		}
		ImGui::Separator();
		if (ImGui::CollapsingHeader("BRDFs Editor")){
			
			if (ImGui::TreeNode("Loaded BRDFs")){ // Loaded BRDFs
				for (auto& it : m_loadedBrdfs)
				{
					ImGui::SetWindowFontScale(1.2);
					if (ImGui::TreeNode(it.first.c_str()))
					{
						// Starting the section of the object
						float bs = ImGui::GetFrameHeight();
						float w = ImGui::GetColumnWidth();
						float len = (it.second.log_e.size() > 0) ? 12.0f : 10.0f;

						ImGui::BeginChild(it.first.c_str(), ImVec2(0.0f, bs * len), true);

						//ImGui::Text("struct BRDF_Output{ vec3 diffuse; vec3 specular;}");

						ImGui::SetWindowFontScale(1.3);
						it.second.glslPanel.Render(it.first.c_str(), ImVec2(w, bs * 8.0f), true);
						ImGui::SetWindowFontScale(1.0);
						ImGui::NewLine();

						if (it.second.glslPanel.IsTextChanged())
							it.second.tested = false;
						
						/*Buttons rendering*/
						ImVec4 col = (it.second.tested) ? ImVec4(0, 0.7, 0, 1) : ImVec4(0.7, 0, 0, 1);
						ImGui::PushStyleColor(ImGuiCol_Button , col);
						if(!it.second.tested) ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col);
						ImGui::SameLine(0, w / 20);
						bool test = ImGui::Button("Test", ImVec2(w / 4, 0));
						ImGui::SameLine(0, w / 20);
						bool push = ImGui::Button("View", ImVec2(w / 4, 0));
						ImGui::SameLine(0, w / 20);
						bool save = ImGui::Button("Save", ImVec2(w / 4, 0));
						ImGui::PopStyleColor();
						if (!it.second.tested) ImGui::PopStyleColor();
						
						/*Logging the errors of the Test*/
						if (it.second.log_e.size() > 0) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
							ImGui::TextWrapped(it.second.log_e.c_str());
							ImGui::PopStyleColor();
						}

						if (test) {
							std::string concat;
							std::string brdf_s = it.second.glslPanel.GetText();
							concat.reserve(brdf_s.length() + m_mainFragShader.length());
							for (int i = 0; i < m_mainFragShader.size(); i++)
								concat = (m_mainFragShader[i] == '\0') ? concat : concat + m_mainFragShader[i];
							for (int i = 0; i < brdf_s.size(); i++)
								concat = (brdf_s[i] == '\0') ? concat : concat + brdf_s[i];

							/*Compile the concatenated fragment shader.*/
							try {
								auto frag_spirv = compileShader(concat, false, it.second.brdfName);
								it.second.tested = true;
								it.second.log_e = "";
								it.second.latest_spir_v = frag_spirv;
							}
							catch (const std::exception& exp) {
								it.second.log_e = exp.what();
								it.second.tested = false;
								//std::cout << "[ERROR]: Compilation Error for the given shader" << std::endl;
							}
						}
						if (push && it.second.tested) {
							recreatePipeline(it.second.brdfName, it.second.latest_spir_v);
						}

						if (save && it.second.tested) {
							saveBRDF(it.second.brdfName, true);
						}

						ImGui::EndChild();
						ImGui::TreePop();
					}					
				}
				ImGui::TreePop();
			}// end Loaded Brdfs

			ImGui::Separator();

			if (ImGui::TreeNode("Costum BRDFs")){ // Costum BRDFs
				std::vector<std::string> deletedInd;
				for (auto& it : m_costumBrdfs){ // Costum brdfs nodes renderer
					ImGui::SetWindowFontScale(1.2);
					if (ImGui::TreeNode(it.first.c_str()))
					{
						// Starting the section of the object
						float bs = ImGui::GetFrameHeight();
						float w = ImGui::GetColumnWidth();

						float len = (it.second.log_e.size() > 0) ? 12.0f : 10.0f;
						ImGui::BeginChild(it.first.c_str(), ImVec2(0.0f, bs* len), true);

						/*Code window*/
						ImGui::SetWindowFontScale(1.3);
						it.second.glslPanel.Render(it.first.c_str(), ImVec2(w, bs * 8.0f), true);
						ImGui::SetWindowFontScale(1.0);
						ImGui::NewLine();
						if (it.second.glslPanel.IsTextChanged())
							it.second.tested = false;

						/*Buttons rendering*/
						ImVec4 col = (it.second.tested) ? ImVec4(0, 0.7, 0, 1) : ImVec4(0.7, 0, 0, 1);
						ImGui::SameLine(0, w / 9);
						ImGui::PushStyleColor(ImGuiCol_Button, col);
						bool test = ImGui::Button("Test", ImVec2(w / 3, 0));
						ImGui::PopStyleColor();
						ImGui::SameLine(0, w / 9);
						bool add = ImGui::Button("Add", ImVec2(w / 3, 0));

						/*Logging the errors of the Test*/
						if (it.second.log_e.size() > 0) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
							ImGui::TextWrapped(it.second.log_e.c_str());
							ImGui::PopStyleColor();
						}

						if (test) {
							std::string concat;
							std::string brdf_s = it.second.glslPanel.GetText();
							concat.reserve(brdf_s.length() + m_mainFragShader.length());
							for (int i = 0; i < m_mainFragShader.size(); i++)
								concat = (m_mainFragShader[i] == '\0') ? concat : concat + m_mainFragShader[i];
							for (int i = 0; i < brdf_s.size(); i++)
								concat = (brdf_s[i] == '\0') ? concat : concat + brdf_s[i];

							/*Compile the concatenated fragment shader.*/
							try {
								auto frag_spirv = compileShader(concat, false, it.second.brdfName);
								it.second.tested = true;
								it.second.log_e = "";
								it.second.latest_spir_v = frag_spirv;
							}
							catch (const std::exception& exp) {
								it.second.tested = false;
								it.second.log_e = exp.what();
								//std::cout << "[ERROR]: Compilation Error for the given shader" << std::endl;
							}
						}
						if (add && it.second.tested) {
							m_loadedBrdfs.insert({ it.first, it.second });
							addPipeline(it.second.brdfName, it.second.latest_spir_v);
							deletedInd.push_back(it.first);
						}

						ImGui::EndChild();
						ImGui::TreePop();
					}
				}// costum BRDFs Nodes rendering

				/*Clearing the costum list if they are added to the loaded brdfs*/
				for (std::string& s : deletedInd) m_costumBrdfs.erase(s);

				/*Adding a new costum BRDF to the system*/
				static char name[20];
				if (strlen(name) == 0) memset(name, '\0', 20);
				bool pressedB = ImGui::Button("+");
				ImGui::SameLine();
				ImGui::InputText("Name", name , 20);
				if (pressedB && strlen(name) > 0) {
					BRDF_Panel panel;
					panel.brdfName = name;
					panel.glslPanel.SetText("vec3 render(vec3 L, vec3 N, vec3 V, vec2 textureCord, mat3 worldToLocal){\n\n}");
					panel.tested = false;
					
					memset(name, '\0', 20);
					std::cout << "The new name is: " << panel.brdfName << std::endl;
					bool unique = m_loadedBrdfs.find(panel.brdfName) == m_loadedBrdfs.end() && m_costumBrdfs.find(panel.brdfName) == m_costumBrdfs.end();
					if (unique) m_costumBrdfs.insert({ panel.brdfName, panel });
				}

				ImGui::TreePop();
			} // Costum BRDFs
		}

		ImGui::SetWindowFontScale(1.0);
		ImGui::End();
	}


	void BRDFA_Engine::drawUI_logger(){
		if (!m_uistate.logWindowActive) return;
		static VkPresentModeKHR presentMode = chooseSwapPresentMode(querySwapChainSupport(m_device).presentModes);
		static int fps = 0;
		static int lastfps = fps;
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto endTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(endTime - startTime).count();
		if (time >= 1.0f) {
			startTime = endTime;
			lastfps = fps;
			fps = 0;
		}

		static std::string logger = "";

		ImGuiWindowFlags file_reader_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
		ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Appearing);
		ImGui::Begin("Logs Window", &m_uistate.logWindowActive, file_reader_flags);

		/*Times elapsed per frame render*/
		ImGui::Text("Time Per Frame: %.2f ms", m_uistate.timePerFrame);

		/*Frames per second*/
		ImGui::Text("Frames Per Second: %d", lastfps);

		/*Vertices Count*/
		uint32_t vsum = this->m_skymap_mesh.vertices.size();
		for (const auto& mesh : this->m_meshes) {
			vsum += mesh.vertices.size();
		}
		ImGui::Text("Vertices Count: %d vertices", vsum);

		/*Current Rendering mode*/
		switch (presentMode) {
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			ImGui::Text("Buffer Strategy: Immediate");
			break;
		case VK_PRESENT_MODE_MAILBOX_KHR:
			ImGui::Text("Buffer Strategy: Mailbox");
			break;
		case VK_PRESENT_MODE_FIFO_KHR:
			ImGui::Text("Buffer Strategy: FIFO");
			break;
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			ImGui::Text("Buffer Strategy: FIFO Relaxed");
			break;
		case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
			ImGui::Text("Buffer Strategy: Shared Demand Refresh");
			break;
		case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
			ImGui::Text("Buffer Strategy: Shared Continous Refresh");
			break;
		default:
			ImGui::Text("Buffer Strategy: Undefined");
			break;
		}

		ImGui::End();
		fps++;
	}


	void BRDFA_Engine::drawUI_comparer(){
		if (!m_uistate.brdfCompareWindowActive)
			return;
	}


	void BRDFA_Engine::drawUI_frameSaver(uint32_t imageIndex){
		static char buf[50];

		if (!m_uistate.frameSaverWindowActive) {
			memset(buf, '\0', 50);
			return;
		}

		ImGui::SetNextWindowPos(ImVec2(this->m_configuration.width / 3, 0), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(this->m_configuration.width / 4, 0), ImGuiCond_Appearing);
		ImGui::Begin("Frame Saver Window", &m_uistate.frameSaverWindowActive, ImGuiWindowFlags_NoResize);

		static int totalRegisteredFrames = 0;	// Current brdfs registered to be rendered.
		ImVec2 ws = ImGui::GetWindowSize();
		ws.y = 0;

		ImGui::InputText("Save As", buf, 50);
		ImGui::TextWrapped("- Click 'Record' to take a screen shot of the current display");
		ImGui::Separator();
		if (!this->saveShot && ImGui::Button("Record", ws)) {
			this->savedFramesDir = std::string(buf);
			this->saveShot = true;
		}
		ImGui::End();

	}


	/// <summary>
	/// Creates The testing window UI.
	/// </summary>
	void BRDFA_Engine::drawUI_tester(){
		if (!m_uistate.testWindowActive)
			return;

		/*Variables that will be used*/
		static bool testing = false;
		static bool showLog = false;
		ImVec2 ws = ImGui::GetWindowSize();
		float fh = ImGui::GetFrameHeight();
		ws.y = 0.0f;

		/*If show log is tru, then the test log window is open*/
		if (showLog) {
			ImGui::SetNextWindowPos(ImVec2(this->m_configuration.width / 3, 150.0f), ImGuiCond_Appearing);
			ImGui::SetNextWindowSize(ImVec2(this->m_configuration.width / 3, fh * 10.0f), ImGuiCond_Appearing);
			ImGui::Begin("Test Result", &showLog);
			ImGui::BulletText("BRDF Test Logs: ");
			for (const auto& it : m_loadedBrdfs) {
				std::string lg = it.second.brdfName;
				ImGui::Text("\t[%s]:", lg.c_str());
				lg = (it.second.log_e == "") ? std::string("is good and flawless!"): it.second.log_e;
				ImGui::TextWrapped("\t%s\n", lg.c_str());
				ImGui::Separator();
			}
			ImGui::End();
		}

		/*Window inititialization*/
		ImGui::SetNextWindowPos(ImVec2( this->m_configuration.width / 3, 100.0f), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(this->m_configuration.width / 3, 0.0f), ImGuiCond_Always);
		ImGui::Begin("Test Window", &m_uistate.testWindowActive);


		/*The selected brdfs that require testing*/
		if (ImGui::CollapsingHeader("Editting BRDFs")) {
			for (auto& it : m_loadedBrdfs) {
				if (testing && it.second.requireTest)
					ImGui::BulletText("TESTING: %s", it.first.c_str());
				else if(!testing){
					ImGui::Checkbox(it.first.c_str(), &it.second.requireTest);
				}
			}
		}

		ImGui::Separator();

		/*If currently a test is going, draw progress bar and return*/
		if (futurePool.size() > 0 && testing) {
			int cur = 0;
			int sum = futurePool.size();

			/*Check the sum of the completed tests*/
			for (auto& t : futurePool) {
				std::future_status status = t.wait_for(std::chrono::milliseconds(0));
				if (status == std::future_status::ready) {
					cur++;
				}
			}

			/*If all threads had terminated successfully, we terminate them.*/
			if (cur == sum) {
				for (auto& t : futurePool) t.get();
				futurePool.clear();
				testing = false;
				showLog = true;
			}

			float frac = float(cur)/float(sum);
			frac = std::clamp(frac, 0.2f, 1.0f);
			ImGui::ProgressBar(frac, ImVec2(0.0f, 0.0f));

			ImGui::End();
			return;
		}

		/*Test button and testing*/
		if (!testing && futurePool.size() == 0 && ImGui::Button("Test", ws)){
			
			for (auto& it : m_loadedBrdfs) {
				if (!it.second.requireTest) continue;
				testing = true;

				std::string concat;
				std::string brdf_s = it.second.glslPanel.GetText();
				concat.reserve(brdf_s.length() + m_mainFragShader.length());
				for (int i = 0; i < m_mainFragShader.size(); i++)
					concat = (m_mainFragShader[i] == '\0') ? concat : concat + m_mainFragShader[i];
				for (int i = 0; i < brdf_s.size(); i++)
					concat = (brdf_s[i] == '\0') ? concat : concat + brdf_s[i];

				/*Compiling the code asynchronysly*/
				BRDF_Panel& lp = it.second;
				auto loadedBRDFs = &m_loadedBrdfs;
				futurePool.push_back(
					std::async(std::launch::async, [concat, &lp, loadedBRDFs] {
						threadCompileGLSL(concat, lp, loadedBRDFs, true);
						return true;
					})
				);

			}
		}

		//ImGui::ShowDemoWindow();
		ImGui::End();
	}


	/// <summary>
	/// 
	/// </summary>
	void BRDFA_Engine::drawUI_menubar() {
		int h = ImGui::GetFrameHeight();
		int w = 0; //this->m_configuration.width;

		ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
		
		ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		// Rendering the Menu bar										 
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::BeginMenu("Open")) {
					if (ImGui::MenuItem("Object", "Ctrl+O")) {
						m_uistate.objectLoaderWindowActive = true;
						m_uistate.extraTexturesCount = 0;
						memset(m_uistate.obj_path, '\0', 100);
						memset(m_uistate.tex_path, '\0', 100);
						memset(m_uistate.extra_tex_paths[0], '\0', 100);
						memset(m_uistate.extra_tex_paths[1], '\0', 100);
						memset(m_uistate.extra_tex_paths[2], '\0', 100);
					}
					if (ImGui::MenuItem("Skymap", "Ctrl+E")) {
						m_uistate.skymapLoaderWindowActive = true;
						memset(m_uistate.skymap_path, '\0', 100);
					}
					ImGui::EndMenu();
				}
				// if (ImGui::MenuItem("Close")) { m_uistate.running = false; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Objects Editor")) m_uistate.objWindowActive = true;
				if (ImGui::MenuItem("Camera Editor")) m_uistate.camWindowActive = true;
				if (ImGui::MenuItem("BRDF Editor")) m_uistate.brdfEditorWindowActive = true;
				if (ImGui::MenuItem("Log Window")) m_uistate.logWindowActive = true;
				if (ImGui::MenuItem("Test Window")) m_uistate.testWindowActive= true;
				if (ImGui::MenuItem("Frame Saver")) m_uistate.frameSaverWindowActive = true;
				ImGui::EndMenu();
			}
			if ( ImGui::MenuItem("Help"))  m_uistate.helpWindowActive = true;
			ImGui::EndMenuBar();
		}
		ImGui::End();
	}


	/// <summary>
	/// 
	/// </summary>
	void BRDFA_Engine::drawUI_objectLoader(){
		if (!m_uistate.objectLoaderWindowActive)
			return;

		static std::string logger = "";

		ImGuiWindowFlags file_reader_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
		ImGui::SetNextWindowPos(ImVec2(100,100),ImGuiCond_Appearing);

		ImGui::Begin("Object Loader", &m_uistate.objectLoaderWindowActive, file_reader_flags);
		ImGui::InputText("Object path", m_uistate.obj_path, 100, ImGuiInputTextFlags_AlwaysOverwrite);
		ImGui::InputText("iTexture0", m_uistate.tex_path, 100, ImGuiInputTextFlags_AlwaysOverwrite);


		for (int j = 0; j < m_uistate.extraTexturesCount; j++) {
			std::string label = std::string("iTexture") + std::to_string(j + 1);
			ImGui::InputText(label.c_str(), m_uistate.extra_tex_paths[j], 100, ImGuiInputTextFlags_AlwaysOverwrite);
		}

		if (m_uistate.extraTexturesCount < 3 && ImGui::Button("+", ImVec2(50, 0)))
			m_uistate.extraTexturesCount++;


		if (ImGui::Button("Load File", ImVec2(100, 30))) {
			std::vector<std::string> texture_paths;
			texture_paths.resize(m_uistate.extraTexturesCount + 2);
			texture_paths[0] = std::string(m_uistate.tex_path);
			for (int j = 0; j < m_uistate.extraTexturesCount; j++)
				texture_paths[j + 1] = std::string(m_uistate.extra_tex_paths[j]);

			try {
				if (!this->loadObject(std::string(m_uistate.obj_path), texture_paths)) 
					logger = "Object can't be loaded: Make sure to have iTexture0 filled and object path is correct!";			
				else {
					m_uistate.objectLoaderWindowActive = false;
					logger = "";
				}
			}
			catch (const std::exception& exp) {
				logger = exp.what();
			}
		}

		if (logger.size() > 0) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
			ImGui::TextWrapped(logger.c_str());
			ImGui::PopStyleColor();
		}
		
		ImGui::End();
	
	}
	

	/// <summary>
	/// 
	/// </summary>
	void BRDFA_Engine::drawUI_skymapLoader(){
		if (!m_uistate.skymapLoaderWindowActive)
			return;

		static std::string logger = "";

		ImGuiWindowFlags file_reader_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
		ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Appearing);
		ImGui::Begin("Skybox loader", &m_uistate.skymapLoaderWindowActive, file_reader_flags);
		ImGui::InputText("Skymap path", m_uistate.skymap_path, 100, ImGuiInputTextFlags_AlwaysOverwrite);
		if (ImGui::Button("Load File", ImVec2(100, 30))) {
			try {
				this->reloadSkymap(std::string(m_uistate.skymap_path));
				logger = "";
				m_uistate.skymapLoaderWindowActive = false;
			}
			catch (const std::exception& exp){
				logger = exp.what();
			}
		}
		if (logger.size() > 0) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
			ImGui::TextWrapped(logger.c_str());
			ImGui::PopStyleColor();
		}
		ImGui::End();
	}

}