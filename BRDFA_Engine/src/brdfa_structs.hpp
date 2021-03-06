#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include <optional>
#include <array>
#include <unordered_map>

// GLM Dependencies
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <imgui_text_editor/TextEditor.h>

#include <cmath>


#include <glfw/glfw3.h>


#include <iostream>

namespace brdfa {

    /// <summary>
    /// Describe the rendering options that the engine provides. It holds all the variaty of BRDFs that we going to analyze.
    /// </summary>
    enum RenderOption {
        BRDFA_TEXTURE = 0,            // Texture rendering
        BRDFA_NORMALS = 1,            // Normals of the mesh
        BRDFA_VIEW_ANGLE = 2,            // dot(Normal, View angle)
        BRDFA_REFLECTION_ANGLE = 3,            // dot(Normal, Reflection Direction)
        BRDFA_DIFFUSE = 4,            // 100% diffuse
        BRDFA_REFECTION = 5,            // 100% reflection
        BRDFA_COOKTORRANCE = 6,            // Naive cooktorrance model
        BRDFA_PHONG = 7,            // basic phong model
        BRDFA_MAX_OPTIONS = 8,            // Indecates the maximum length of the options
    };




    /// <summary>
    /// Holds the states of the UI buttons and options. For now we only have the rendering option.
    /// </summary>
    struct UIState {
        char    obj_path[100];				// Mesh path
        char    tex_path[100];				// Texture path
        char    extra_tex_paths[3][100];	// Texture path
        int     extraTexturesCount = 0;     // The number of extra textures.
        char    skymap_path[100];		    // Skymap texture


        bool    running = true;
        bool    focused = true;

        /*File loading windows*/
        bool    objectLoaderWindowActive = false;
        bool    skymapLoaderWindowActive = false;
        
        /*Tools windows*/
        bool    logWindowActive = false;
        bool    objWindowActive = false;
        bool    helpWindowActive = false;
        bool    camWindowActive = false;
        bool    testWindowActive = false;
        bool    brdfEditorWindowActive = false;
        bool    brdfCompareWindowActive = false;
        bool    frameSaverWindowActive = false;

        float   timePerFrame = 0.0;
        
    };


    /// <summary>
    /// Holds the device stuff and its specific vulkan objects.
    /// </summary>
    struct Device {
        VkSampleCountFlagBits			msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPhysicalDevice                physicalDevice;                 // GPU Vulkan object (Id to gpu device)
        VkDevice                        device;                         // Logical device vulkan object.
        VkSurfaceKHR					surface;                     // Presentation surface
        VkQueue                         graphicsQueue;
        VkQueue                         presentQueue;
    };


    /// <summary>
    /// Image holds all the data needed to create an image or sampler.
    /// </summary>
    struct Image {
        bool                            cubemap = false;                // If the image represents a cube map or not.
        VkImage                         obj;                            // Image object handled by Vulkan.
        VkDeviceMemory                  memory;                         // Memory ID of the allocated Image on the device. Vulkan handles these kind of IDs
        VkImageView                     view;                           // The Image view attached to the Image object.
        VkSampler                       sampler = VK_NULL_HANDLE;       // Incase the image needs to be sampled and sent to the GPU.
        uint32_t                        mipLevels;                      // Miplevels count of the image.
        uint32_t                        width, height;
    };


    /// <summary>
    /// 
    /// </summary>
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
        VkDescriptorSetLayout           layout = VK_NULL_HANDLE;                         // The layout of the descriptor sets
        VkDescriptorPool                pool = VK_NULL_HANDLE;                           // Vulkan Object which handles memory allocation functionalities
        std::vector<VkDescriptorSet>    sets;                           // The sets we have initialized and created.
    };


    struct GPipeline {
        VkRenderPass                    uiRenderpass;                   // Render pass for the UI
        VkRenderPass                    sceneRenderPass;                // Render pass to be used in Graphics pipeline.
        VkPipelineLayout                layout;                         // Pipeline layout used in the current Graphics pipeline.
        std::unordered_map<std::string, VkPipeline>                      pipelines;                       // Graphics pipeline that we can submit commands into.
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
        alignas(16) glm::vec3           pos_c;                          // Camera position in the world
        alignas(16) glm::vec3           render_opt;                     // This holds the rendering option, roughness, specularity and other data that are sent to the gpu.
    };


    
    struct Parameters {
        alignas(16) glm::vec3           extra012 = glm::vec3(0.0, 0.0, 0.0);       // Extra parameters from 0 to 2
        alignas(16) glm::vec3           extra345 = glm::vec3(0.0, 0.0, 0.0);       // Extra parameters from 3 to 5
        alignas(16) glm::vec3           extra678 = glm::vec3(0.0, 0.0, 0.0);       // Extra parameters from 6 to 8
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
        glm::vec3 normal;


        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

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

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, normal);

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const {
            return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
        }
    };



    struct Mesh {
        uint32_t					uid;
        std::vector<Image>			textureImages;				        // Holds the texture Image data.

        std::vector<Vertex>			vertices;					        // Vertices of the Mesh. Vertices can hold more than a position.
        std::vector<uint32_t>		indices;					        // Indices refering to the loaded vertices of the object.

        float                       extra[2] = { 0, 0 };
        int                         samples = 100;

        glm::vec3                   rotation = glm::vec3(0,0,0);        // Rotation of the object.
       //  glm::vec3                   scale = glm::vec3(1);               // Holds the scale of the object along the axis
        glm::vec3                   translation = glm::vec3(0,0,0);
        // glm::mat4                   transformation = glm::mat4(1.f);   // Holds the object transformation. Object to World transformation.
        glm::vec3                   scale = glm::vec3(1, 1, 1);         // Holds the object scale on x,y,z

        Parameters                  params = {};                        // Parameters regarding this object
        std::vector<Buffer>         paramsBuffer;                       // Holds the parameter buffers
        int                         shownParameters = 0;                // The number of the shown extra parameters

        std::string                 renderOption = "None";

        Buffer						vertexBuffer;				        // Vulkan buffer of the vertices
        Buffer						indexBuffer;				        // Vulkan buffer of the Indices


        glm::mat4 getFinalTransformation() {
            glm::mat4 ret = glm::mat4(1.f);
            ret = glm::translate(ret, translation);
            

            ret = glm::rotate(ret, glm::radians(rotation[0]), glm::vec3(1, 0, 0));
            ret = glm::rotate(ret, glm::radians(rotation[1]), glm::vec3(0, 1, 0));
            ret = glm::rotate(ret, glm::radians(rotation[2]), glm::vec3(0, 0, 1));
            
            ret = glm::scale(ret, scale);

            return ret;
        }


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



    /// <summary>
    /// 
    /// </summary>
    struct MouseEvent {
        glm::vec2 init_cords = glm::vec2(0.0f, 0.0f);
        glm::vec2 delta_cords = glm::vec2(0.0f, 0.0f);
        bool update = false;
    };




    struct Camera {
        uint32_t                        uid;                        // For future implementation.
        glm::mat4                       transformation;             // Camera to world space transformation matrix.
        glm::mat4                       projection;                 // Projection matrix.
        float                           aspectRatio;                // Camera aspect ratio: W/H
        float                           nPlane, fPlane;             // Near and Far clipping plane
        float                           angle;                      // Angle of the camera y-axis (Height)
        float                           speed_t = 5.f, speed_r = 1.f;                // Camera movement speed.

        glm::vec3                       rotation = glm::vec3(0.0f);     // Holdes the accumalated rotation of the camera.
        glm::vec3                       position = glm::vec3(0.0f);     // Holds the current position of the camera. 
        glm::vec3                       direction = glm::vec3(0.0f, 0.0f, -1.0f);    // Look direction.


        Camera() {}

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
            rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
            position = glm::vec3(0.0f, 0.0f, 50.0f);


            direction.x = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
            direction.y = sin(glm::radians(rotation.y));
            direction.z = sin(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
            direction = glm::normalize(direction);

            updateViewMatrix();

            projection = glm::perspective(glm::radians(angle), aspectRatio, nPlane, fPlane);
            projection[1][1] *= -1;
        }

        /// <summary>
        /// Used to print a matrix to the standard output.
        /// </summary>
        /// <param name="M"></param>
        void printTransformation() {

            for (int i = 0; i < 4; i++) {
                std::cout << "[";
                for (int j = 0; j < 4; j++) {
                    std::string space = (j == 3) ? "" : ", ";
                    std::cout << transformation[i][j] << space;
                }
                std::cout << "]" << std::endl;
            }

        }


        void updateViewMatrix() {
            transformation = glm::lookAt(this->position, this->position + this->direction, glm::vec3(0.0f, 1.0f, 0.0f));
        }


        void updateDirection() {
            direction.x = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
            direction.y = sin(glm::radians(rotation.y));
            direction.z = sin(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
            direction = glm::normalize(direction);
        }


        void update(const KeyEvent& ke, float time, float translationSpeed, float rotationSpeed) {
            /*Zooming:*/
            if (ke.key == GLFW_KEY_E && ke.action != GLFW_RELEASE)
                this->position += this->direction * time * translationSpeed;
            if (ke.key == GLFW_KEY_Q && ke.action != GLFW_RELEASE)
                this->position -= this->direction * time * translationSpeed;

            updateViewMatrix();
#ifdef _DEBUG
            std::cout << "New Matrix transformation of the Camera is: " << std::endl;
            printTransformation();
            std::cout << std::endl;
#endif

        }



        /// <summary>
        /// 
        /// </summary>
        /// <param name="me"></param>
        /// <param name="time"></param>
        /// <param name="translationSpeed"></param>
        /// <param name="rotationSpeed"></param>
        void update(const KeyEvent& ke, MouseEvent& me, float time) {


            /*Forward, backward*/
            if (ke.key == GLFW_KEY_W && ke.action != GLFW_RELEASE)
                this->position += this->direction * time * speed_t;
            if (ke.key == GLFW_KEY_S && ke.action != GLFW_RELEASE)
                this->position -= this->direction * time * speed_t;

            /*right, left*/
            if (ke.key == GLFW_KEY_D && ke.action != GLFW_RELEASE) {
                glm::vec3 rightDir = glm::cross(this->direction, glm::vec3(0, 1, 0));
                this->position += rightDir * time * speed_t;
                //this->position += glm::vec3(this->transformation[0][0], this->transformation[0][1], this->transformation[0][2]) * time * translationSpeed;
            }
            if (ke.key == GLFW_KEY_A && ke.action != GLFW_RELEASE) {
                glm::vec3 rightDir = glm::cross(this->direction, glm::vec3(0, 1, 0));
                this->position -= rightDir * time * speed_t;
            }

            /*up, down*/
            if (ke.key == GLFW_KEY_Q && ke.action != GLFW_RELEASE) {
                glm::vec3 rightDir = glm::cross(this->direction, glm::vec3(0, 1, 0));
                glm::vec3 upDir = glm::cross(rightDir, this->direction);
                this->position += upDir * time * speed_t;
            }
            if (ke.key == GLFW_KEY_E && ke.action != GLFW_RELEASE) {
                glm::vec3 rightDir = glm::cross(this->direction, glm::vec3(0, 1, 0));
                glm::vec3 upDir = glm::cross(rightDir, this->direction);
                this->position -= upDir * time * speed_t;
            }



            if (me.update) {
                glm::vec2 temp = me.delta_cords * time * speed_r;
                temp.y *= -1.0f;    // For flipping the movement.
                rotation += glm::vec3(temp, 0.0f);


                /*Restricting the angle of horizontal movement.*/
                if (rotation.y > 89.0f)
                    rotation.y = 89.0f;
                if (rotation.y < -89.0f)
                    rotation.y = -89.0f;

                /*Recalculation of the matrices. */
                /* glm::mat4 rotM = glm::mat4(1.0f);
                rotM = glm::rotate(rotM, glm::radians(temp.y), glm::vec3(1.0f, 0.0f, 0.0f));
                rotM = glm::rotate(rotM, glm::radians(temp.x), glm::vec3(0.0f, 1.0f, 0.0f));*/
                direction.x = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
                direction.y = sin(glm::radians(rotation.y));
                direction.z = sin(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
                direction = glm::normalize(direction);


                /*Rotating the direction vector.*/
                //this->direction = glm::normalize(glm::vec3(rotM * glm::vec4(this->direction, 1.0f))); 
            }

            updateViewMatrix();
#ifdef _DEBUG
            if (me.update) {
                std::cout << "New Matrix transformation of the Camera is: " << std::endl;
                printTransformation();
                printf("Camera position: (%f, %f, %f)\n\n", position.x, position.y, position.z);
                printf("Camera direction: (%f, %f, %f)\n\n", direction.x, direction.y, direction.z);
                std::cout << std::endl;
            }
#endif
        }
    };




    /// <summary>
    /// Holds the needed data for a singular brdf panel in the menu. It stores if the code has been changed and stuff.
    /// </summary>
    struct BRDF_Panel{
        std::string             brdfName;
        TextEditor              glslPanel;	/*IMGUI Text editor Addon: https://github.com/ELTE-IK-CG/Dragonfly/tree/master/include/ImGui-addons/imgui_text_editor*/
        std::vector<char>       latest_spir_v;
        std::string             log_e = "";            // Holds the log of the last compilation
        bool                    tested = false;
        bool                    requireTest = false;
        bool                    saveFrame = false;      // Used for saving the frame of the current brdf
    };

    
}