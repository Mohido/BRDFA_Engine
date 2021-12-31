#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm/glm.hpp>
#include <optional>
#include <array>

namespace brdfa {
    struct Device {
        VkSampleCountFlagBits			msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPhysicalDevice                physicalDevice;                 // GPU Vulkan object (Id to gpu device)
        VkDevice                        device;                         // Logical device vulkan object.
        VkSurfaceKHR					surface;                     // Presentation surface
        VkQueue                         graphicsQueue;
        VkQueue                         presentQueue;
    };


    struct Image {
        VkImage                         obj;                            // Image object handled by Vulkan.
        VkDeviceMemory                  memory;                         // Memory ID of the allocated Image on the device. Vulkan handles these kind of IDs
        VkImageView                     view;                           // The Image view attached to the Image object.
        VkSampler                       sampler = VK_NULL_HANDLE;       // Incase the image needs to be sampled and sent to the GPU.
        uint32_t                        mipLevels;                      // Miplevels count of the image.
        uint32_t                        width, height;
    };

    struct SwapChain {
        VkSwapchainKHR                  swapChain;                      // Swapchain vulkan object (ID to a swapchain)
        std::vector<VkImage>            images;                         // Swapchain Image objects.
        VkFormat                        format;                         // Swapchain image format type
        VkExtent2D                      extent;                         // Swapchain window size
        std::vector<VkImageView>        imageViews;                     // Image views to render into
        std::vector<VkFramebuffer>      framebuffers;                   // framebuffers to hold different attachments through the render pass.
        Image							colorImage;					    // A color resolve attachment for miltisampling
        Image							depthImage;		    			// A depth attachment used for depth testing.

    };

    struct Descriptor {
        VkDescriptorSetLayout           layout  = VK_NULL_HANDLE;                         // The layout of the descriptor sets
        VkDescriptorPool                pool    = VK_NULL_HANDLE;                           // Vulkan Object which handles memory allocation functionalities
        std::vector<VkDescriptorSet>    sets;                           // The sets we have initialized and created.
    };


    struct GPipeline {
        VkRenderPass                    uiRenderpass;                   // Render pass for the UI
        VkRenderPass                    sceneRenderPass;                // Render pass to be used in Graphics pipeline.
        VkPipelineLayout                layout;                         // Pipeline layout used in the current Graphics pipeline.
        VkPipeline                      pipeline;                       // Graphics pipeline that we can submit commands into.
    };


    struct Commander {
        VkCommandPool                   pool;                           // Handles the memory allocation of the command buffers
        std::vector<VkCommandBuffer>    sceneBuffers;                   // Command buffers allocated from this pool.
        std::vector<VkCommandBuffer>    uiBuffers;
    };




    struct Buffer {
        VkBuffer                        obj;                            // Vulkan Object ID. Vulkan don't allocate memory for the buffer.
        VkDeviceMemory                  memory;                         // The allocated Memory ID. It is somekind of a pointer that Vulkan understands.
    };


    struct MVPMatrices {
        alignas(16) glm::mat4           model;                          // Model matrix: Maps model to world space.
        alignas(16) glm::mat4           view;                           // View matrix: Maps object to camera space
        alignas(16) glm::mat4           proj;                           // Projection matrix 
    };


    struct SyncCollection {
        VkSemaphore                     s_imageAvailable;
        VkSemaphore                     s_renderFinished;
        VkFence                         f_inFlight;
    };


    struct Instance {
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
    };


    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };



    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };


    struct Mesh {
        uint32_t						uid;
        Image							textureImage;				// Holds the texture Image data.

        std::vector<Vertex>				vertices;					// Vertices of the Mesh. Vertices can hold more than a position.
        std::vector<uint32_t>			indices;					// Indices refering to the loaded vertices of the object.

        Buffer							vertexBuffer;				// Vulkan buffer of the vertices
        Buffer							indexBuffer;				// Vulkan buffer of the Indices
    };
}
