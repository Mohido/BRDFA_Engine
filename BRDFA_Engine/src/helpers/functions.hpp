#pragma once

#include <brdfa_structs.hpp>
#include <brdfa_cons.hpp>
#include <brdfa_callbacks.hpp>


#include <algorithm>
#include <vulkan/vulkan.h>
#include <iostream>


namespace brdfa {

    enum BoxSide {
        RIGHT = 0, 
        LEFT = 1, 
        TOP = 2, 
        BOTTOM = 3,
        FRONT = 4,
        BACK = 5
    };


    /// <summary>
    /// 
    /// </summary>
    /// <param name="instance"></param>
    /// <param name="pCreateInfo"></param>
    /// <param name="pAllocator"></param>
    /// <param name="pDebugMessenger"></param>
    /// <returns></returns>
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="createInfo"></param>
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);


    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    bool checkValidationLayerSupport();


    /// <summary>
    /// 
    /// </summary>
    /// <param name="enableValidationLayers"></param>
    /// <returns></returns>
    std::vector<const char*> getRequiredExtensions(const bool& enableValidationLayers);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="applicationName"></param>
    /// <param name="enableValidationLayers"></param>
    /// <param name="instance"></param>
    void createInstance(const char* applicationName, const bool& enableValidationLayers, Instance& instance);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <param name="typeFilter"></param>
    /// <param name="properties"></param>
    /// <returns></returns>
    uint32_t findMemoryType(const Device& device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="physicalDevice"></param>
    /// <returns></returns>
    VkSampleCountFlagBits getMaxUsableSampleCount(const VkPhysicalDevice& physicalDevice);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <param name="surface"></param>
    /// <returns></returns>
    QueueFamilyIndices findQueueFamilies(const Device& device);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    bool checkDeviceExtensionSupport(const VkPhysicalDevice& device);




    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    SwapChainSupportDetails querySwapChainSupport(const Device& device);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    bool isDeviceSuitable(const Device& device);


    /// <summary>
    /// 
    /// </summary>
    /// <param name="instance"></param>
    /// <param name="surface"></param>
    /// <param name="device"></param>
    /// <param name="queues"></param>
    void pickPhysicalDevice(const VkInstance& instance, Device& device);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <param name="enableValidationLayers"></param>
    void createLogicalDevice(Device& device, const bool& enableValidationLayers);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="availablePresentModes"></param>
    /// <returns></returns>
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);




    /// <summary>
    /// 
    /// </summary>
    /// <param name="image"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="format"></param>
    /// <param name="oldLayout"></param>
    /// <param name="newLayout"></param>
    void transitionImageLayout(
        Image& image, 
        Commander& commander, 
        const Device& device,
        VkFormat format, 
        VkImageLayout oldLayout, 
        VkImageLayout newLayout);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="capabilities"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <returns></returns>
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const uint32_t& width, const uint32_t& height);




    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="image"></param>
    /// <param name="imageFormat"></param>
    /// <param name="texWidth"></param>
    /// <param name="texHeight"></param>
    void generateMipmaps(
        Commander& commander, const Device& device, 
        Image& image, VkFormat imageFormat, 
        int32_t texWidth, int32_t texHeight);


    /// <summary>
    /// 
    /// </summary>
    /// <param name="availableFormats"></param>
    /// <returns></returns>
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);


    /// <summary>
    /// 
    /// </summary>
    /// <param name="image"></param>
    /// <param name="format"></param>
    /// <param name="aspectFlags"></param>
    /// <param name="mipLevels"></param>
    /// <returns></returns>
    VkImageView createImageView(
        VkImage image, const VkDevice& device, 
        VkFormat format, VkImageAspectFlags aspectFlags, 
        uint32_t mipLevels, bool cubemap = false);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="image"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    void createColorResources(Image& image, Commander& commander, const Device& device, const SwapChain& swapchain);


    /// <summary>
    /// 
    /// </summary>
    /// <param name="image"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    void createDepthResources(Image& image, Commander& commander, const Device& device, const SwapChain& swapchain);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="swapchain"></param>
    /// <param name="colorImage"></param>
    /// <param name="depthImage"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="gpipeline"></param>
    void createFramebuffers(SwapChain& swapchain, Commander& commander, const Device& device, const GPipeline& gpipeline);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    void createSwapChain(SwapChain& swapchain, const Device& device, const uint32_t& width, const uint32_t& height);



    //////////////////////////////////////////////  Graphics Pipeline Abstractions ////////////////////////////////////////////////////


    /// <summary>
    /// 
    /// </summary>
    /// <param name="filename"></param>
    /// <returns></returns>
    std::vector<char> readFile(
        const std::string& filename, 
        const bool& binary = true);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="code"></param>
    /// <returns></returns>
    VkShaderModule createShaderModule(
        const Device& device, 
        const std::vector<char>& code);


    /// <summary>
    /// Given a glsl code, it compiles it at runtime and returns a spir-v code.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="glslCode"></param>
    /// <returns></returns>
    std::vector<char> compileShader(
        const std::string& glslCode, 
        const bool& vertexShader = true, 
        const std::string& shadername = "realtimeShader");



    /// <summary>
    /// 
    /// </summary>
    /// <param name="gpipeline"></param>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    void createRenderPass(
        GPipeline& gpipeline, 
        const Device& device, 
        const SwapChain& swapchain);


    /// <summary>
    /// This will allow the creation of a pipeline layout
    /// </summary>
    /// <param name="gpipeline"></param>
    /// <param name="device"></param>
    /// <param name="descriptor"></param>
    void createPipelineLayout(
        GPipeline& gpipeline, 
        const Device& device, 
        const Descriptor& descriptor);


    /// <summary>
    /// 
    /// </summary>
    /// <param name="gpipeline"></param>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    /// <param name="descriptor"></param>
    void createGraphicsPipeline(
        const VkPipelineLayout& layout, 
        const VkRenderPass& sceneRenderPass,
        VkPipeline& gpipeline, 
        VkPipeline& sky_map_pipeline,
        const Device& device, 
        const SwapChain& swapchain,
        const Descriptor& descriptor, 
        const std::vector<char>& vertShaderSpirv,
        const std::vector<char>& fragShaderSpirv, 
        const bool& isSkymap);




    ///////////////////////////////////////////////////////////////////// Commanders Abstractions




    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    void endSingleTimeCommands(
        Commander& commander, 
        const Device& device);



    /// <summary>
    /// The commander buffers will act as a stack in this function.
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <returns></returns>
    VkCommandBuffer beginSingleTimeCommands(
        Commander& commander, 
        const Device& device);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <param name="mipLevels"></param>
    /// <param name="numSamples"></param>
    /// <param name="format"></param>
    /// <param name="tiling"></param>
    /// <param name="usage"></param>
    /// <param name="properties"></param>
    /// <param name="image"></param>
    /// <param name="imageMemory"></param>
    void createImage(
        const Commander& commander,
        const Device& device, 
        uint32_t width, 
        uint32_t height, 
        uint32_t mipLevels,
        VkSampleCountFlagBits numSamples, 
        VkFormat format, 
        VkImageTiling tiling, 
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties, 
        Image& image, 
        bool cubemap = false, 
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);




    /// <summary>
    /// 
    /// </summary>
    /// <param name="size"></param>
    /// <param name="usage"></param>
    /// <param name="properties"></param>
    /// <param name="buffer"></param>
    /// <param name="bufferMemory"></param>
    void  createBuffer(
        Commander& commander, 
        const Device& device, 
        VkDeviceSize size, 
        VkBufferUsageFlags usage, 
        VkMemoryPropertyFlags properties, 
        Buffer& buffer);




    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="srcBuffer"></param>
    /// <param name="dstBuffer"></param>
    /// <param name="size"></param>
    void copyBuffer(
        Commander& commander, 
        const Device& device, 
        VkBuffer srcBuffer, 
        VkBuffer dstBuffer, 
        VkDeviceSize size);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="buffer"></param>
    /// <param name="image"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    void copyBufferToImage(
        Commander& commander, 
        const Device& device, 
        Buffer& buffer, 
        Image& image, 
        uint32_t width, 
        uint32_t height);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="commandPool"></param>
    /// <param name="device"></param>
    void createCommandPool(
        VkCommandPool& commandPool,
        const Device& device);



    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="gpipeline"></param>
    /// <param name="swapchain"></param>
    /// <param name="index"></param>
    void updateUICommandBuffers(
        Commander& commander, 
        const Device& device, 
        const GPipeline& gpipeline,
        const SwapChain& swapchain, 
        const uint32_t index);


    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="gpipeline"></param>
    /// <param name="descriptorObj"></param>
    /// <param name="swapchain"></param>
    /// <param name="meshes"></param>
    void recordCommandBuffers(
        Commander& commander, 
        const Device& device, 
        const GPipeline& gpipeline, 
        const Descriptor& descriptorObj, 
        const SwapChain& swapchain, 
        std::vector<Mesh>& meshes, 
        Mesh& skymap, 
        VkPipeline& skymap_pipeline);


    ////////////////////////////////////////////////////////////////// Descriptors Abstractions



    /// <summary>
    /// 
    /// </summary>
    /// <param name="descriptorObj"></param>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    void createDescriptorSetLayout(
        Descriptor& descriptorObj, 
        const Device& device, 
        const SwapChain& swapchain);


    /// <summary>
    /// creates a BRDFA Descriptor Object. 
    /// Descriptor Object holds the sets being created, the pool which they correspond to, and
    /// their layout in memory. Number of sets is determined by: swapchain images size * meshes count in the scene.
    /// </summary>
    /// <param name="descriptorObj">The desired Descriptor Object to be filled</param>
    /// <param name="device">BRDFA Device object</param>
    /// <param name="swapchain">BRDFA SwapChain object</param>
    /// <param name="meshCount">Number of meshes needed to be rendered</param>
    void initDescriptors(
        Descriptor& descriptorObj, 
        const Device& device, 
        const SwapChain& swapchain, 
        const std::vector<Buffer>& uniformBuffers, 
        std::vector<Mesh>& meshes, 
        Image& skymap);


    /////////////////////////////////////////////////// Mesh abstractions



    /// <summary>
    /// 
    /// </summary>
    /// <param name="mesh"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="modelPath"></param>
    void loadVertices(
        Mesh& mesh, 
        Commander& commander, 
        const Device& device, 
        const std::string& modelPath);


    /// <summary>
    /// 
    /// </summary>
    /// <param name="texturePath"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    void loadTexture(
        Mesh& mesh, 
        Commander& commander, 
        const Device& device, 
        const std::string& texturePath);




    /// <summary>
    /// 
    /// </summary>
    /// <param name="mesh"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="modelPath"></param>
    /// <param name="texturePath"></param>
    void populate(
        Mesh& mesh, 
        Commander& commander, 
        const Device& device, 
        const std::string& modelPath, 
        const std::string& texturePath);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="modelPath"></param>
    /// <param name="texturePath"></param>
    /// <returns></returns>
    Mesh loadMesh(
        Commander& commander, 
        const Device& device, 
        const std::string& modelPath, 
        const std::string& texturePath, 
        const size_t& bufferCounts);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="modelPath"></param>
    /// <param name="texturePath"></param>
    /// <returns></returns>
    Mesh loadMesh(
        Commander& commander, 
        const Device& device, 
        const std::string& modelPath, 
        const std::vector<std::string>& texturePaths, 
        const size_t& bufferCounts);


    /// <summary>
    /// 
    /// </summary>
    /// <param name="mesh"></param>
    /// <param name="device"></param>
    void destroyMesh(
        Mesh& mesh, 
        const Device& device);


    /////////////////////////////////////////////////// Extrea 

    /// <summary>
    /// Returns the data of one of the skymap sides. 
    /// </summary>
    /// <param name="image"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <param name="face"></param>
    /// <returns></returns>
    char* loadFace(
        char* image, 
        const int& width, 
        const int& height, 
        const BoxSide& face);


    /// <summary>
    /// 
    /// </summary>
    /// <param name="syncs"></param>
    /// <param name="imagesInFlight"></param>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    /// <param name="maxFramesInFlight"></param>
    void createSyncObjects(
        std::vector<SyncCollection>& syncs, 
        std::vector<VkFence>& imagesInFlight, 
        const Device& device, 
        const SwapChain& swapchain, 
        uint8_t maxFramesInFlight);


    /// <summary>
    /// Creates Empty buffers in the RAM for holding Uniform variables. 
    ///     
    /// Note:
    ///     Each scene object will have its own related uniform buffer.
    ///     Each independent frame will have its own related uniform buffer.
    ///     Therefore, Uniform buffers size is: swapchain images * mesh count.
    /// </summary>
    /// <param name="uniformBuffers"></param>
    /// <param name="commander"></param>
    /// <param name="device"></param>
    /// <param name="swapchain"></param>
    /// <param name="meshCount"></param>
    void createUniformBuffers(
        std::vector<Buffer>& uniformBuffers, 
        Commander& commander, 
        const Device& device, 
        const SwapChain& swapchain,
        uint32_t meshCount);



    void threadAddSpirv(
        const std::string& cacheFileName,
        BRDF_Panel lp, 
        std::unordered_map<std::string, BRDF_Panel>* loadedBRDFs);


    void threadCompileGLSL(
        const std::string& concat, 
        BRDF_Panel& lp, 
        std::unordered_map<std::string, BRDF_Panel>* loadedBRDFs, 
        bool testing = false);


}