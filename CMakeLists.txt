cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 17)

project (cgin)

add_executable(cgin
 
 
"source/AccessSpecifier.h" 
"source/AssetManager.cpp" 
"source/AssetManager.h" 
"source/Buffer.cpp" 
"source/Buffer.h" 
 
 
"source/Camera.cpp" 
"source/Camera.h" 
 
 
"source/ClearPass.cpp" 
"source/ClearPass.h" 
 
 
"source/ComputePass.cpp" 
"source/ComputePass.h" 
"source/CoordinateSystem.h" 
"source/DrawPass.cpp" 
"source/DrawPass.h" 
 
"source/GeometryContainer.cpp" 
"source/GeometryContainer.h" 
 
"source/Image.cpp" 
"source/Image.h" 
"source/InputSupport.cpp" 
"source/InputSupport.h" 
"source/main.cpp" 
"source/Pass.cpp" 
"source/Pass.h" 
"source/PassDependencyManager.cpp" 
"source/PassDependencyManager.h" 
"source/PipelinePass.cpp" 
"source/PipelinePass.h" 
"source/PixelDataContainer.cpp" 
"source/PixelDataContainer.h" 
"source/PresentationController.cpp" 
"source/PresentationController.h" 
"source/PresentPass.cpp" 
"source/PresentPass.h" 
 
 
"source/Resource.cpp" 
"source/Resource.h" 
"source/ResourceAccessSpecifier.h" 
"source/Scene.h" 
"source/Shader.cpp" 
"source/Shader.h" 
"source/SwapChain.cpp" 
"source/SwapChain.h" 
"source/Timer.cpp" 
"source/Timer.h" 
 
 
 
"source/Transform.cpp" 
"source/Transform.h" 
 
"source/VulkanCore.cpp" 
"source/VulkanCore.h" 
 

 "source/WorkContainer.h" "source/WorkContainer.cpp" "source/Behavior.h" "source/Behavior.cpp")

find_package(Vulkan REQUIRED)

file(GLOB SHADER_SOURCES
	"${PROJECT_SOURCE_DIR}/assets/shaders/*.vert" 
	"${PROJECT_SOURCE_DIR}/assets/shaders/*.frag" 
	"${PROJECT_SOURCE_DIR}/assets/shaders/*.comp"
)

foreach(SHADER_SOURCE ${SHADER_SOURCES})
	get_filename_component(FILE_NAME ${SHADER_SOURCE} NAME)
	set(SHADER_SPIRV "${PROJECT_SOURCE_DIR}/assets/shaders/${FILE_NAME}.spv")
	add_custom_command(
		OUTPUT ${SHADER_SPIRV}
		COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER_SOURCE} -o ${SHADER_SPIRV}
		DEPENDS ${SHADER_SOURCE}
		)
		list(APPEND SHADER_SPIRVS ${SHADER_SPIRV})
endforeach(SHADER_SOURCE)

target_include_directories(cgin PUBLIC ${Vulkan_INCLUDE_DIR})
target_include_directories(cgin PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_directories(cgin PUBLIC ${PROJECT_SOURCE_DIR}/lib)

target_link_libraries(cgin ${Vulkan_LIBRARY})
target_link_libraries(cgin glfw3)

add_custom_target(shaders DEPENDS ${SHADER_SPIRVS})

add_dependencies(cgin shaders)
