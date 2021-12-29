#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <string>
#include <array>
#include <vector>

#include "brdfa_structs.hpp"



namespace brdfa {


	// ----------------------------- Configuration Struct ------------------------------------------

	struct BRDFAEngineConfiguration {
		uint16_t						width, height;						// Window initial width and height.
		uint16_t						ups = 60;							// Updates per second (30, 60 or 120).
		std::vector<std::string>		shadersPaths;						// Shaders file paths. Relative to the Project path.
		bool							validationLayersEnabled;			// Enable Validation layers for logging.

	};


	// ----------------------------- BRDFA Engine ------------------------------------------

	class BRDFA_Engine {
	private:
		/*Engine configuration*/
		const BRDFAEngineConfiguration		m_configuration;				// the engine initial configuration.
		size_t								m_currentFrame = 0;				// The current frame being rendered.
		bool								m_active;
		
		uint32_t m_width_w, m_height_w;	


		/*GFLW stuff*/
		GLFWwindow*							m_window;						// window handler.
		bool								m_frameBufferResized;			// If frame buffer is resized or not.

		/*Vulkan objects*/
		Instance							m_instance;                     // Vulkan process handler
        Device								m_device;                       // Holds the physical/logical device of the engine.
        SwapChain							m_swapChain;                    // swapchain related objects.
        Descriptor							m_descriptorData;               // Holds Descriptor pool and its relative layout and sets.
		GPipeline							m_graphicsPipeline;				// Holds the Graphics pipeline data.
		Commander							m_commander;					// Handles the command pool and its related command buffers.

		std::vector<SyncCollection>			m_sync;							// Fences per swapchain image. CPU/GPU signals, Semaphores per swapchain image. GPU/GPU signals.
		std::vector<VkFence>				m_imagesInFlight;
		
		/*Engine Scene variables*/
		std::vector<Mesh>					m_meshes;						// Scene meshes.
		std::vector<Buffer>					m_uniformBuffers;				// Scene Uniform buffers. Camera transformation is a such.

		const uint8_t						MAX_FRAMES_IN_FLIGHT = 2;


	public:
		BRDFA_Engine(const BRDFAEngineConfiguration& conf)
			: m_configuration(conf), m_frameBufferResized(false)
		{

		}

		void close();

		~BRDFA_Engine();

		bool init();													// Initialize the window and the vulkan api

		bool interrupt();												// interrupt execution .. For later usage.

		bool loadObject();												// loading a mesh object into the scene.

		bool updateAndRender();											// main loop. It handles engine updates and rendering.

		bool isClosed();												// if the engine is closed.

		bool frameBufferResize();										// set the frame buffer resized flag to true.

	private:
		void startWindow();
		void startVulkan();
		void update(uint32_t currentImage);
		void cleanup();
		void recreate();
	};
}
