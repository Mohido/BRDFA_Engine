#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <imgui_text_editor/TextEditor.h>

#include <string>
#include <array>
#include <queue>
#include <vector>
#include <thread>
#include <future>
#include "brdfa_structs.hpp"



namespace brdfa {


	// ----------------------------- Configuration Struct ------------------------------------------

	struct BRDFAEngineConfiguration {
		uint16_t						width, height;						// Window initial width and height.
		uint16_t						ups = 60;							// Updates per second (30, 60 or 120).
		std::vector<std::string>		shadersPaths;						// Shaders file paths. Relative to the Project path.
		bool							validationLayersEnabled;			// Enable Validation layers for logging.
		bool							hot_load = false;
		bool							no_cache_load = false;
	};


	// ----------------------------- BRDFA Engine ------------------------------------------

	class BRDFA_Engine {
	private:
		/*Imgui stuff*/
		bool											m_activeImgui = true;
		VkDescriptorPool								m_imguiPool;

		/*Engine configuration*/
		const BRDFAEngineConfiguration					m_configuration;				// the engine initial configuration.
		size_t											m_currentFrame = 0;				// The current frame being rendered.
		bool											m_active;
		uint32_t										m_width_w, m_height_w;	
		// std::unordered_map<std::string, std::string>	m_code_loaded;
		


		/*GFLW stuff*/
		GLFWwindow*										m_window;						// window handler.
		bool											m_frameBufferResized;			// If frame buffer is resized or not.

		/*Vulkan objects*/
		Instance										m_instance;                     // Vulkan process handler
        Device											m_device;                       // Holds the physical/logical device of the engine.
        SwapChain										m_swapChain;                    // swapchain related objects.
        Descriptor										m_descriptorData;               // Holds Descriptor pool and its relative layout and sets.
		GPipeline										m_graphicsPipelines;				// Holds the Graphics pipeline data.
		Commander										m_commander;					// Handles the command pool and its related command buffers.
		std::vector<SyncCollection>						m_sync;							// Fences per swapchain image. CPU/GPU signals, Semaphores per swapchain image. GPU/GPU signals.
		std::vector<VkFence>							m_imagesInFlight;
		


		/*Engine Scene variables*/
		Camera											m_camera;
		std::vector<Mesh>								m_meshes;						// Scene meshes.
		std::vector<Buffer>								m_uniformBuffers;				// Scene Uniform buffers. Camera transformation is a such.
		Mesh											m_skymap_mesh;					// Mesh that defines the skymap to be rendered. It is rendered on a seperate pipeline
		Image											m_skymap;						// Skybox image
		VkPipeline										m_skymap_pipeline;				// Pipeline that holds the Skymap Shaders info.


		/*Event System.*/
		KeyEvent										m_keyboardEvent;				// Events per updates.
		MouseEvent										m_mouseEvent;					// Holds the values require by the mouse event.


		/*UI state system*/
		UIState											m_uistate;
		std::unordered_map<std::string, BRDF_Panel>		m_loadedBrdfs;
		std::unordered_map<std::string, BRDF_Panel>		m_costumBrdfs;


		std::string										m_mainFragShader;
		std::vector<char>								m_vertSpirv;

		const uint8_t									MAX_FRAMES_IN_FLIGHT = 2;

		/*Parallalism utilitities*/
		std::vector<std::thread>						compilationPool;				// Holds all the threads that are being used to compile the glsl at the moment.
		std::vector<std::future<bool>>					futurePool;						// Another threading utility for helping us create threads that return values

		/*saving images Utilities.*/
		std::string										savedFramesDir = "res/";		// The frames Directory.
		bool											saveShot = false;

	public:
		BRDFA_Engine(const BRDFAEngineConfiguration& conf)
			: m_configuration(conf), m_frameBufferResized(false)
		{

		}

		~BRDFA_Engine();					

		bool init();														// Initialize the window and the vulkan api

		bool updateAndRender();												// main loop. It handles engine updates and rendering.

		void close();														// closes the engine. By default the engine will call this funciton when the window is closed

		bool interrupt();													// interrupt execution .. For later usage.

		bool loadObject(const std::string& object_path, const std::vector<std::string>& texture_path);												// loading a mesh object into the scene.
		bool deleteObject(const int& idx);									// Deletes the object corresponding to that index.

		bool reloadSkymap(const std::string& path);

		bool loadEnvironmentMap(const std::array<std::string,6>& skyboxSides);										// Loading an environment map into the scene.
		bool loadEnvironmentMap(const std::string& skyboxSides);										// Loading an environment map into the scene.


		bool isClosed();													// if the engine is closed.

		bool frameBufferResize();											// set the frame buffer resized flag to true.
		
		void fireKeyEvent(int key, int action);								// adds an event.

		void fireMouseButtonEvent(int button, int action);					// adds the event to the mouse buttons

		GLFWwindow* getWindow() { return this->m_window; }					// Returns the glfw window of the engine.

	private:
		//void threadAddSpirv(std::string cacheFileName, BRDF_Panel lp);
		//void threadCompileGLSL(std::string concat, BRDF_Panel lp);

		void drawUI_objects();
		void drawUI_camera();
		void drawUI_editorBRDF();
		void drawUI_logger();
		void drawUI_comparer();
		void drawUI_frameSaver(uint32_t imageIndex);
		void drawUI_tester();
		void drawUI_menubar();
		void drawUI_objectLoader();
		void drawUI_skymapLoader();


		void refreshObject(const size_t& idx);													// Records the objects back again.
		void addFragPipeline(const std::string&, const std::string&);							// This is used to add a pipeline to the scene. And refreshes the obejcts.
		void saveBRDF(const std::string& brdfName, const bool& cacheIt = true);					// Save the BRDF to the disk.
		void recreatePipeline(const std::string&, const std::vector<char>&);					// Quickly recreates a specific pipeline.
		void addPipeline(const std::string&, const std::vector<char>&);							// Add a new pipeline to the graphics pipelines.
		void loadPipelines();																	// Load all pipelines needed by the program to run.
		void startWindow();																		// Starts the GLFW window
		void startVulkan();																		// Fully initialize the Vulkan engine.
		bool startImgui();																		// Starts the Imgui for vulkan and glfw
		void update(uint32_t currentImage);														// Update function. Time dependent function.
		void render(uint32_t imageIndex);														// Render the engine's scene.
		void record(uint32_t imageIndex);														// Save the frame into a file.
		void drawUI(uint32_t imageIndex);														// Draw the UI (ImGui)
		void cleanup();																			// Clean up the swapchain and the Vulkan objects. Mostly used during window resizing
		void recreate();																		// Cleans up the vulkan engine and recreate its objects. Called when the window is being resized.
		//void drawUI_menubar();
	};
}
