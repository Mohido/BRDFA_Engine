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
#include "brdfa_functions.hpp"
#include "brdfa_callbacks.hpp"

// STD Dependencies
#include <iostream>
#include <chrono>
#include <filesystem>





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
	bool BRDFA_Engine::loadObject(const std::string& object_path, const std::string& texture_path) {
		if (object_path.size() == 0 || texture_path.size() == 0) {
			std::cout << "INFO: Object and texture paths must be given to load the model. We don't support texture-less models yet." << std::endl;
			return false;
		}
		vkDeviceWaitIdle(m_device.device);
		
		/*Adding the new mesh*/
		m_meshes.push_back(loadMesh(m_commander, m_device, object_path, texture_path));		// Loading veriaty of objects

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
			throw std::runtime_error("ERROR: failed to load Environment Map image!: " + skyboxSides + " Does not exist!");
			return false; // we should not break the whole program if we don't find an image...
		}
		
		unsigned int faceWidth = texWidth / 4;
		unsigned int faceHeight = texHeight / 3;

		VkDeviceSize imageSize = faceWidth * faceHeight * 4 * 6;				// Full buffer size (The size of all the images.)
		VkDeviceSize layerSize = faceWidth * faceHeight * 4;			// Size per layer

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
	void BRDFA_Engine::recreatePipeline(const std::string& brdfName, const std::vector<char>& fragSpirv)
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
		for (size_t j = 0; j < m_meshes.size(); j++)
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

			printf("[INFO]: Loading file: %s\n", brdfFilePath.c_str());
			int extInd = brdfFileName.find(".brdf");
			bool isbrdf = extInd != std::string::npos;
			if (isbrdf) {
				/*BRDF name as a key and the shader file path.*/
				std::string brdfName(brdfFileName.begin() , brdfFileName.begin() + extInd);
				std::string shaderPath = brdfFilePath + "/" + brdfFileName;

				printf("[INFO]: Loading BRDF: %s\n", brdfFileName.c_str());

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
				std::vector<char> frag_spirv;
				if (m_loadedBrdfs.find(brdfName) == m_loadedBrdfs.end()) {
					BRDF_Panel	lp;
					lp.brdfName = brdfName;
					lp.glslPanel.SetText(brdf_s);
					if (m_configuration.hot_load) {
						lp.tested = false;
						m_loadedBrdfs.insert({ brdfName, lp });
						continue;
					}
					frag_spirv = compileShader(concat, false, "FragmentSHader");
					lp.latest_spir_v = frag_spirv;
					m_loadedBrdfs.insert({ brdfName, lp });
				}

				/*Insert a new pipeline.*/
				m_graphicsPipelines.pipelines.insert({ brdfName , {} });
				createGraphicsPipeline(
					m_graphicsPipelines.layout, m_graphicsPipelines.sceneRenderPass, 
					m_graphicsPipelines.pipelines.at(brdfName), m_skymap_pipeline, 
					m_device, m_swapChain, m_descriptorData, m_vertSpirv, frag_spirv, false);
			}
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
		m_meshes.push_back(loadMesh(m_commander, m_device, MODEL_PATH, TEXTURE_PATH ));		// Loading veriaty of objects
		loadVertices(m_skymap_mesh, m_commander, m_device, CUBE_MODEL_PATH);				// Loading skymap vertices (CUBE)
		loadEnvironmentMap(SKYMAP_PATHS);
		m_camera = Camera(m_swapChain.extent.width, m_swapChain.extent.height, 0.1f, 10.0f, 45.0f);

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
				m_camera.update(m_keyboardEvent, dull, timeDelta, 0.75f, 0.75f);
			}
			else {
				m_mouseEvent.delta_cords = temp;
				m_camera.update(m_keyboardEvent, m_mouseEvent, timeDelta, 0.75f, 0.75f);
			}
		} // end update camera system
		

		for (size_t i = 0; i < m_meshes.size(); i++) { // setup ubos for meshes
			size_t ind = i * m_swapChain.images.size() + currentImage;
			
			MVPMatrices ubo{};
			ubo.model = m_meshes[i].transformation;			//glm::rotate(modelTr, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = m_camera.transformation;				//glm::lookAt(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj = m_camera.projection;					//glm::perspective(glm::radians(45.0f), m_swapChain.extent.width / (float)m_swapChain.extent.height, 0.1f, 10.0f);
			ubo.pos_c = m_camera.position;
			ubo.render_opt = glm::vec3(0.0f, 0.0f, 0.0f);

			void* data;
			vkMapMemory(m_device.device, m_uniformBuffers[ind].memory, 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(m_device.device, m_uniformBuffers[ind].memory);
		}// end setup ubos 
		
		lastTime = currentTime;
	}







	/// <summary>
	/// private member function to render.
	/// </summary>
	/// <param name="imageIndex">The image Index currently being processed. (Inflag image)</param>
	void BRDFA_Engine::render(uint32_t imageIndex) {
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
	}



	/// <summary>
	/// 
	/// </summary>
	/// <param name="imageIndex"></param>
	void BRDFA_Engine::drawUI(uint32_t imageIndex) {
		static char obj_path[100];				// Mesh path
		static char tex_path[100];				// Texture path

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		/*Create the engine menu window*/
		ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_FirstUseEver);
		ImGui::Begin("BRDFA Engine Menu", &m_uistate.running, ImGuiWindowFlags_MenuBar);

		// Rendering the Menu bar
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open..", "Ctrl+O")) { 
					m_uistate.readFileWindowActive = true;
					memset(obj_path, '\0', 100);
					memset(tex_path, '\0', 100);
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
				if (ImGui::MenuItem("Close", "Ctrl+W")) { m_uistate.running = false; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Skymap..", "Ctrl+E")) {
					m_uistate.readSkymapWindowActive = true;
					memset(tex_path, '\0', 100);
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		if (m_uistate.readFileWindowActive) {// Object File loader Window	
			ImGuiWindowFlags file_reader_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
			ImGui::Begin("File Reader", &m_uistate.readFileWindowActive, file_reader_flags);
			ImGui::InputText("Object path", obj_path, 100, ImGuiInputTextFlags_AlwaysOverwrite);
			ImGui::InputText("Texture path", tex_path, 100, ImGuiInputTextFlags_AlwaysOverwrite);
			if (ImGui::Button("Load File", ImVec2(100, 30))) {
				this->loadObject(std::string(obj_path), std::string(tex_path));
				m_uistate.readFileWindowActive = false;
			}
			ImGui::End();
		}// Object File loader Window

		if (m_uistate.readSkymapWindowActive) {// Skymap File loader Window
			ImGuiWindowFlags file_reader_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
			ImGui::Begin("File Reader", &m_uistate.readSkymapWindowActive, file_reader_flags);
			ImGui::InputText("Skymap path", tex_path, 100, ImGuiInputTextFlags_AlwaysOverwrite);
			if (ImGui::Button("Load File", ImVec2(100, 30))) {
				this->reloadSkymap(std::string(tex_path));
				m_uistate.readSkymapWindowActive = false;
			}
			ImGui::End();
		}// Skymap File loader Window
		{
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("WindowTabs", tab_bar_flags))
			{
				if (ImGui::BeginTabItem("Objects"))
				{
					drawUI_objects();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Camera"))
				{
					drawUI_camera();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Advance"))
				{
					drawUI_advance();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::Separator();
		}

		ImGui::End();		// "BRDFA engine menu" end()
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

		/*Clearing the Graphics pipeline*/
		for (auto& it : m_graphicsPipelines.pipelines) {
			vkDestroyPipeline(m_device.device, it.second , nullptr);
		}
		m_graphicsPipelines.pipelines.clear();
		
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
		loadPipelines();
		createFramebuffers(m_swapChain, m_commander, m_device, m_graphicsPipelines);

		/*Meshes dependent*/
		loadEnvironmentMap(SKYMAP_PATHS);
		createUniformBuffers(m_uniformBuffers, m_commander, m_device, m_swapChain, m_meshes.size());
		initDescriptors(m_descriptorData, m_device, m_swapChain, m_uniformBuffers, m_meshes, m_skymap);
		recordCommandBuffers(m_commander, m_device, m_graphicsPipelines, m_descriptorData, m_swapChain, m_meshes, m_skymap_mesh, m_skymap_pipeline);

		/*Syncronization objects re-initialization.*/
		m_imagesInFlight.resize(m_swapChain.images.size(), VK_NULL_HANDLE);
	}



	/// <summary>
	/// Draws the Objects Panel. The user can change the objects parameters here.
	/// </summary>
	void BRDFA_Engine::drawUI_objects() {
		//ImGuiStyle& style = ImGui::GetStyle();
		//float spacing = style.ItemInnerSpacing.x;
		float button_sz = ImGui::GetFrameHeight();

		// Rendering Meshes data. (Per mesh.)
		for (int i = 0; i < m_meshes.size(); i++) {
			std::string curObj = "Object_" + std::to_string(i + 1);
			const char* current_item = m_meshes[i].renderOption.c_str(); //m_uistate.optionLabels[m_meshes[i].renderOption];

			// Starting the section of the object
			ImGui::BeginChild(curObj.data(), ImVec2(0.0f, button_sz * 10.0f), false);

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
			{ // Object Translation option
				float trans[3] = { m_meshes[i].transformation[3][0] , m_meshes[i].transformation[3][1], m_meshes[i].transformation[3][2] };
				ImGui::InputFloat3("Translation", trans);
				m_meshes[i].transformation[3][0] = trans[0];
				m_meshes[i].transformation[3][1] = trans[1];
				m_meshes[i].transformation[3][2] = trans[2];
			} // Object Translation option
			{ // Object Scale option
				float trans[3] = { m_meshes[i].transformation[0][0] , m_meshes[i].transformation[1][1], m_meshes[i].transformation[2][2] };
				ImGui::InputFloat3("Scale", trans);
				m_meshes[i].transformation[0][0] = trans[0];
				m_meshes[i].transformation[1][1] = trans[1];
				m_meshes[i].transformation[2][2] = trans[2];
			} // Object scale option
			ImGui::Separator();
			{ // Object Scale option
				//float trans[3] = { m_meshes[i].transformation[0][0] , m_meshes[i].transformation[1][1], m_meshes[i].transformation[2][2] };
				float iw = ImGui::CalcItemWidth();
				ImGui::PushItemWidth(iw * 0.5);
				float material[2] = { 0.5 };
				ImGui::InputFloat2("Additional Parameters", material);
				int samples = 100;
				ImGui::InputInt("Light Samples", &samples, 1, 10);
				ImGui::PopItemWidth();
			} // Object scale option


			{ // Object deletion button
				ImGui::NewLine();
				if (ImGui::Button("Delete")) {
					this->deleteObject(i);
					std::cout << "Deleting: " << curObj << std::endl;
				}
			} // Object deletion button

			ImGui::EndChild();
		} // For loop over objects
	}


	/// <summary>
	/// Drarws the Camera panel. The user can edit the camera parameters such as movement speed, zoom, rotate, 
	/// and change the projection matrix
	/// </summary>
	void BRDFA_Engine::drawUI_camera() {
	}


	/// <summary>
	/// Draws the Advance panel. In advance panel, the user can insert new GLSL BRDFs, Test/Compile/Edit/Save them. 
	///		
	/// </summary>
	void BRDFA_Engine::drawUI_advance() {
		// ImGui::ShowDemoWindow();

		if (ImGui::CollapsingHeader("BRDFs Configuration")){

			/*Draw Loaded BRDFs*/
			ImGui::SetWindowFontScale(1.2);
			if (ImGui::TreeNode("Loaded BRDFs")){ // Loaded BRDFs
				for (auto& it : m_loadedBrdfs)
				{
					if (ImGui::TreeNode(it.first.c_str()))
					{
						// Starting the section of the object
						float bs = ImGui::GetFrameHeight();
						float w = ImGui::GetColumnWidth();
						ImGui::BeginChild(it.first.c_str(), ImVec2(0.0f, bs * 11.0f), true);

						ImGui::Text("struct BRDF_Output{ vec3 diffuse; vec3 specular;}");

						ImGui::SetWindowFontScale(1.4);
						it.second.glslPanel.Render(it.first.c_str(), ImVec2(w, bs * 8.0f), true);
						ImGui::SetWindowFontScale(1.0);
						ImGui::NewLine();

						if (it.second.glslPanel.IsTextChanged())
							it.second.tested = false;
						
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
								auto frag_spirv = compileShader(concat, false, "FragmentSHader");
								it.second.tested = true;
								it.second.latest_spir_v = frag_spirv;
							}
							catch (std::exception e) {
								it.second.tested = false;
								std::cout << "[ERROR]: Compilation Error for the given shader" << std::endl;
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
				for (auto& it : m_costumBrdfs){
					if (ImGui::TreeNode(it.first.c_str()))
					{
						// Starting the section of the object
						float bs = ImGui::GetFrameHeight();
						float w = ImGui::GetColumnWidth();
						ImGui::BeginChild(it.first.c_str(), ImVec2(0.0f, bs * 11.0f), true);

						ImGui::Text("struct BRDF_Output{ vec3 diffuse; vec3 specular;}");

						ImGui::SetWindowFontScale(1.4);
						it.second.glslPanel.Render(it.first.c_str(), ImVec2(w, bs * 8.0f), true);
						ImGui::SetWindowFontScale(1.0);
						ImGui::NewLine();

						if (it.second.glslPanel.IsTextChanged())
							it.second.tested = false;

						ImVec4 col = (it.second.tested) ? ImVec4(0, 0.7, 0, 1) : ImVec4(0.7, 0, 0, 1);

						ImGui::SameLine(0, w / 9);
						ImGui::PushStyleColor(ImGuiCol_Button, col);
						bool test = ImGui::Button("Test", ImVec2(w / 3, 0));
						ImGui::PopStyleColor();
						ImGui::SameLine(0, w / 9);
						bool add = ImGui::Button("Add", ImVec2(w / 3, 0));

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
								auto frag_spirv = compileShader(concat, false, "FragmentSHader");
								it.second.tested = true;
								it.second.latest_spir_v = frag_spirv;
							}
							catch (std::exception e) {
								it.second.tested = false;
								std::cout << "[ERROR]: Compilation Error for the given shader" << std::endl;
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
				}

				/*Clearing the costum ones.*/
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
					panel.glslPanel.SetText("BRDF_Output brdf(vec3 L, vec3 N, vec3 V){\n\n}");
					panel.tested = false;
					
					memset(name, '\0', 20);
					std::cout << "The new name is: " << panel.brdfName << std::endl;
					bool unique = m_loadedBrdfs.find(panel.brdfName) == m_loadedBrdfs.end() && m_costumBrdfs.find(panel.brdfName) == m_costumBrdfs.end();
					if (unique) m_costumBrdfs.insert({ panel.brdfName, panel });
				}

				ImGui::TreePop();
			} // Costum BRDFs

			ImGui::SetWindowFontScale(1.0);
		}

		/*TODO:: Implement Collapsing window.*/
		if (ImGui::CollapsingHeader("Tests and Logs Configuration"))
		{

		}
	}
}