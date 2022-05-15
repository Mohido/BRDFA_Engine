# BRDFA_Engine
A 3D rendering engine created for the purpose of allowing the users to write shader code at run-time without the need to restart the engine. The engine harnesses the powers of the GPU through *VulkanAPI* (Rendering API), *GLSL* (Shading Language), and *DearImGui* (Graphical User Interface). And, it provides a set of tools elaborated in details in the thesis paper work (Chapter 2, BRDFA_Engine). Yet, a breif explanations is provided below.

The latest release build can be downloaded from <a href="https://drive.google.com/drive/folders/1zfADjLSjjRJnLnusHOE8dmpnk3qB_yTb?usp=sharing">*here*</a>. Still, an explanation on how to build the engine is provided as well. 

## Integrated Tools
* **Object Loader:** allows the user to load 3D Objects to to the scene. The objects must be *.obj* format.
* **Skymap Loader:** allows the user to change the environment map. Only *Cubemaps* are supported!
* **Object Viewer/Editor:** allows the user to edit the 3D objects settings, such as rotation.
* **Camera Editor:** allows the user to manipulate the camera settings, such as the navigation speed.
* **BRDF Editor:** allows the user to create/edit/test BRDFs and shading functions in general.
* **Log Window:** provides the runtime status of the program, such as the vertices count in the scene.
* **Test Window:** provides an asynchronous testing method for the implemented shading functions through the *BRDF Editor*.
* **Frame Saver:** takes a screenshot of the current rendered frame and saves it as a *.bmp* file.

## Build Recipe
The developer must have the needed libraries installed on their machines before attempting to build the engine. The libraries can be put in the *libs* folder, or their paths can be saved in their default System Environment Variables. The needed libraries are:
* **<a href="https://www.glfw.org/">GLFW</a>** (glfw3.lib)
* **<a href="https://www.lunarg.com/vulkan-sdk/">Vulkan</a>** (shaderc_combined.lib, vulkan-1/vulkan).


All the other third-party source files are included in the *utils* directory to ease with the building steps.

To build the engine, open the engine directory in **Visual Studio 2019** and click on the root CMakeLists.txt file. After that, press `ctrl+s` or click on the build button on the top for the current document. Some of the CMakeLists.txt variables might need to be changed if the libraries are stored in a custom directory and not included in the system environment variables.


## Credits
* *<a href="http://casual-effects.com/data/index.html">McGuire Computer Graphics Archive</a>*
* *<a href="https://github.com/ELTE-IK-CG/Dragonfly/tree/master/include/ImGui-addons/imgui_text_editor">DearImGui Text Editor</a>*
* *<a href="https://github.com/ocornut/imgui">DearImGui</a>*
* *<a href="https://github.com/Groovounet/glm">glm</a>*
* *<a href="https://www.glfw.org/">glfw</a>*
* *<a href="https://github.com/tinyobjloader/tinyobjloader">tinyObjectLoader</a>*
* *<a href="https://github.com/nothings/stb">stb_image & stb_image_write</a>*
* *<a href="https://www.lunarg.com/vulkan-sdk/">Vulkan</a>*