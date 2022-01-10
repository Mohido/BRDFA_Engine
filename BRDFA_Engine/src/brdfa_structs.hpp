#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include <optional>
#include <array>


// GLM Dependencies
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <cmath>

#include <iostream>


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

        glm::mat4                       transformation;             // Holds the object transformation. Object to World transformation.
        Buffer							vertexBuffer;				// Vulkan buffer of the vertices
        Buffer							indexBuffer;				// Vulkan buffer of the Indices
    };





    struct KeyEvent {
        int key;
        int action;
        /*bool            key_w;
        bool            key_s;
        bool            key_a;
        bool            key_d;
        bool            key_shift;

        void clear() {
           key_w = false;
           key_s = false;
           key_a = false;
           key_d = false;
           key_shift = false;
        }*/
    };


    struct Camera {
        uint32_t                        uid;                        // For future implementation.
        glm::mat4                       transformation;             // Camera to world space transformation matrix.
        glm::mat4                       projection;                 // Projection matrix.
        float                           aspectRatio;                // Camera aspect ratio: W/H
        float                           nPlane, fPlane;             // Near and Far clipping plane
        float                           angle;                      // Angle of the camera y-axis (Height)

        Camera(){}

        /// <summary>
        /// Creates a prespective camera entity. 
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="nPlane"></param>
        /// <param name="fPlane"></param>
        /// <param name="yAngle">Angle in degrees of the y-axis (height)</param>
        Camera(uint32_t width, uint32_t height, float nPlane, float fPlane, float yAngle)
        : uid(1)
        {
            aspectRatio = width / (float)height;
            nPlane = nPlane;
            fPlane = fPlane;
            angle = yAngle;

            transformation = glm::lookAt(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            projection = glm::perspective(glm::radians(angle), aspectRatio, nPlane, fPlane);
            projection[1][1] *= -1;
        }

        
        void update(const KeyEvent& ke, float time, float translationSpeed, float rotationSpeed) {

            /*Translation*/
            /*Y Axis*/
            if (ke.key == GLFW_KEY_W && ke.action != GLFW_RELEASE)
                transformation[3][1] += -translationSpeed * time;
            if (ke.key == GLFW_KEY_S && ke.action != GLFW_RELEASE)
                transformation[3][1] += translationSpeed * time;
            /*X Axis*/
            if (ke.key == GLFW_KEY_D && ke.action != GLFW_RELEASE)
                transformation[3][0] += -translationSpeed * time;
            if (ke.key == GLFW_KEY_A && ke.action != GLFW_RELEASE)
                transformation[3][0] += translationSpeed * time;
            /*Z-axis*/
            if(ke.key == GLFW_KEY_E && ke.action != GLFW_RELEASE)
                transformation[3][2] += -translationSpeed * time;
            if (ke.key == GLFW_KEY_Q && ke.action != GLFW_RELEASE)
                transformation[3][2] += translationSpeed * time;

            /*Rotation*/
            /*Initial rotation states.*/

            float phi   = 0.0f;
            /*Y Axis*/
            if (ke.key == GLFW_KEY_I && ke.action != GLFW_RELEASE)
                phi += -rotationSpeed;
            if (ke.key == GLFW_KEY_K && ke.action != GLFW_RELEASE)
                phi += rotationSpeed;
            transformation = glm::rotate(transformation, time * glm::radians(phi), glm::vec3(1.0f, 0.0f, 0.0f));

            /*X Axis*/
            float theta = 0.0f;
            if (ke.key == GLFW_KEY_J && ke.action != GLFW_RELEASE)
                theta += -rotationSpeed;
            if (ke.key == GLFW_KEY_L && ke.action != GLFW_RELEASE)
                theta += rotationSpeed;
            transformation = glm::rotate(transformation, time * glm::radians(theta), glm::vec3(0.0f, 0.0f, 1.0f));

            /*Z Axis*/
            float delta = 0.0f;
            if (ke.key == GLFW_KEY_U && ke.action != GLFW_RELEASE)
                delta += -rotationSpeed;
            if (ke.key == GLFW_KEY_O && ke.action != GLFW_RELEASE)
                delta += rotationSpeed;
            transformation = glm::rotate(transformation, time * glm::radians(delta), glm::vec3(0.0f, 1.0f, 0.0f));
            
            /*Forming rotation matrices*/
      /*      glm::mat4 R_y = glm::mat4(0.0f);
            if (theta != 0.0f) {
                R_y[0][0] = std::cos(theta);
                R_y[2][0] = std::sin(theta);
                R_y[1][1] = 1;
                R_y[0][2] = -std::sin(theta);
                R_y[2][2] = std::cos(theta);
            }
            glm::mat4 R_x = glm::mat4(0.0f);
            if (phi != 0.0f) {
                R_x[1][1] = std::cos(phi);
                R_x[1][2] = -std::sin(phi);
                R_x[0][0] = 1;
                R_x[2][1] = std::sin(phi);
                R_x[2][2] = std::cos(phi);
            }*/

           
        }
    };



}

///*Y Axis*/
//if (ke.key_w && !ke.key_shift)
//    transformation[3][1] += -speed * time;
//if (ke.key_s && !ke.key_shift)
//    transformation[3][1] += speed * time;

///*X Axis*/
//if (ke.key_a)
//    transformation[3][0] += -speed * time;
//if (ke.key_d)
//    transformation[3][0] += speed * time;

///*Z-axis*/
//
//if (ke.key_shift) {
//    std::cout << ke.key_w << " : " << ke.key_shift << " keyshift working" << std::endl;
//    transformation[3][2] += -speed * time;
//   
//}
//    
//if (ke.key_s && ke.key_shift) {
//    std::cout << ke.key_w << " : " << ke.key_shift << " keyshift working" << std::endl;
//    transformation[3][2] += speed * time;
//}