#pragma once


namespace brdfa {
	struct RenderAPIConfig {
		int width, height;
		bool validationLayerEnabled = true;
		uint8_t maxFramesInFlight = 2;
	};


	enum RenderAPIType {
		RENDER_API_VULKAN,
		RENDER_API_OPENGL
	};


	/// <summary>
	/// Abstract class inwhich all application apis must inherit from.
	/// </summary>
	class RenderAPI {
	public:
		RenderAPI(RenderAPIConfig config, RenderAPIType type)
			: m_config(config), m_type(type)
		{}

		virtual ~RenderAPI() {};
		virtual bool init() = 0;							// Initialize the API
		virtual bool render() = 0;							// A render call
		virtual bool shutdown() = 0;						// Shuts down the API

		RenderAPIType getType() {return m_type;}
		RenderAPIConfig getConfig() { return m_config; }
	protected:
		RenderAPIConfig m_config;								// Configuration on the rendering API
		RenderAPIType	m_type = RENDER_API_VULKAN;				// Type of the rendering API
	};
}
