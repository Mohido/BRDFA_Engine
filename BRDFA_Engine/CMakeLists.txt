# CMakeList.txt : CMake project for BRDFA_Engine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8) # Cmake 3.8
set (CMAKE_CXX_STANDARD 17) # C++ 17

# Project name and .o file
project ("BRDFA_Engine")



# IMGUI source files
file (GLOB IMGUI_SRC 
			"utils/imgui/*.cpp" 
			"utils/imgui/*.h"
			)


# IMGUI source files
file (GLOB IMGUI_SRC_ADDONS
			"utils/imgui_text_editor/*.cpp" 
			"utils/imgui_text_editor/*.h"
			)


# Engine Source files
file ( GLOB BRDFA_ENGINE_SRC 
			"src/brdfa*.hpp" 
			"src/brdfa*.cpp"
			"src/helpers/*.hpp"
			"src/helpers/*.cpp"
			)

# Our built executable.
add_executable (BRDFA_Engine "src/main.cpp" ${BRDFA_ENGINE_SRC} ${IMGUI_SRC} ${IMGUI_SRC_ADDONS}  "src/helpers/extra.cpp" "src/brdfa_callbacks.cpp")

# ---------------------------- C++ "Include" directories
include_directories(src) 
include_directories(utils)


# ---------------------------- Loading Shaders to the out folder
file(COPY shaders DESTINATION "${CMAKE_BINARY_DIR}/")
file(COPY res DESTINATION "${CMAKE_BINARY_DIR}/")

# ---------------------------- Needed for SaschaWillems code for loading the VULKAN API
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(NAME BRDFA_Engine)
OPTION(USE_D2D_WSI "Build the project using Direct to Display swapchain" OFF)
OPTION(USE_DIRECTFB_WSI "Build the project using DirectFB swapchain" OFF)
OPTION(USE_WAYLAND_WSI "Build the project using Wayland swapchain" OFF)
OPTION(USE_HEADLESS "Build the project using headless extension swapchain" OFF)
set(RESOURCE_INSTALL_DIR "" CACHE PATH "Path to install resources to (leave empty for running uninstalled)")

# Use FindVulkan module added with CMAKE 3.7. Mine was one: C:/VulkanSDK/1.2.189.2/Lib/vulkan-1.lib
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	message(STATUS "Using module to find Vulkan")
	find_package(Vulkan)
endif()



IF(UNIX AND NOT APPLE)
	set(LINUX TRUE)
ENDIF()

IF(WIN32)
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()
	#find_library(shaderc_LIBRARY NAMES shaderc_combinedd HINTS "$ENV{VULKAN_SDK}/lib/shaderc_combinedd.lib" "${CMAKE_SOURCE_DIR}/libs/shaderc_combinedd.lib" REQUIRED)
	find_library(shaderc_d_LIBRARY NAMES shaderc_combinedd HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/shaderc_combinedd.lib" REQUIRED)
	find_library(shaderc_LIBRARY NAMES shaderc_combined HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/shaderc_combined.lib" REQUIRED)
	message("[win32] vulkan library: ${Vulkan_LIBRARY}")
	message("[win32] shaderc library: ${shaderc_d_LIBRARY}")
	message("[win32] shaderc library: ${shaderc_LIBRARY}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSEIF(LINUX)
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
		find_library(shaderc_LIBRARY NAMES shaderc_combined HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			set(shaderc_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()
	find_package(Threads REQUIRED)
	IF(USE_D2D_WSI)
		MESSAGE("Using direct to display extension...")
		add_definitions(-D_DIRECT2DISPLAY)
	ELSEIF(USE_DIRECTFB_WSI)
		find_package(DirectFB REQUIRED)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_DIRECTFB_EXT")
		include_directories(${DIRECTFB_INCLUDE_DIR})
	ELSEIF(USE_WAYLAND_WSI)
		find_program(PKG_CONFIG pkg-config)
		if (NOT PKG_CONFIG)
			message(FATAL_ERROR "pkg-config binary not found")
		endif ()
		find_package(Wayland REQUIRED)
		if (NOT WAYLAND_FOUND)
			message(FATAL_ERROR "Wayland development package not found")
		endif ()
		pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
		if (NOT WAYLAND_PROTOCOLS_FOUND)
			message(FATAL_ERROR "Wayland protocols package not found")
		endif ()
		find_program(WAYLAND_SCANNER wayland-scanner)
		if (NOT WAYLAND_SCANNER)
			message(FATAL_ERROR "wayland-scanner binary not found")
		endif ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR")
		include_directories(${WAYLAND_INCLUDE_DIR})
		pkg_get_variable(protocol_dir wayland-protocols pkgdatadir)
		execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
				COMMAND ${WAYLAND_SCANNER} private-code ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
		include_directories(${CMAKE_BINARY_DIR})
	ELSEIF(USE_HEADLESS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_HEADLESS_EXT")
	ELSE(USE_D2D_WSI)
		find_package(XCB REQUIRED)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
	ENDIF(USE_D2D_WSI)
ELSEIF(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_MACOS_MVK -DVK_EXAMPLE_XCODE_GENERATED")
	# Todo : android?
ENDIF(WIN32)

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message("FOUND libraries")
	message(STATUS ${Vulkan_LIBRARY})
	message(STATUS ${shaderc_LIBRARY})
ENDIF()

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# Clang specific stuff
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
endif()
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB SOURCE *.cpp )

# Compiler specific stuff
IF(MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ELSEIF(APPLE)
	if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -ObjC++")
	ELSE()
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -xobjective-c++")
	ENDIF()
ENDIF(MSVC)

IF(WIN32)
	message("Linked: ${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${shaderc_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}" )
	link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${shaderc_d_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ELSEIF(APPLE)
	link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
ELSE(WIN32)
	message("Linked:")
	link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${shaderc_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(WIN32)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(SHADERC_ENABLE_SHARED_CRT ON)


# ------------------------      Link libraries libraries
IF (Vulkan_FOUND)
	MESSAGE("Using bundled Vulkan library version, ${Vulkan_LIBRARY}")
	MESSAGE("Using bundled Shaderc library version, ${shaderc_LIBRARY}")
	target_link_libraries(BRDFA_Engine "${CMAKE_SOURCE_DIR}/libs/glfw3.lib")
	target_link_libraries(BRDFA_Engine "${shaderc_d_LIBRARY}")
	target_link_libraries(BRDFA_Engine "${Vulkan_LIBRARY}")
ENDIF()

# ------------------------      Adding subdirectories. 
add_subdirectory(src)
add_subdirectory(utils)
