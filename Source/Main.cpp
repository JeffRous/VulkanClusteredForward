#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include <tiny_obj_loader.h>
/*view https://github.com/syoyo/tinyobjloader for more informations*/

#include "Application/Application.h"

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan Clustered Forward", nullptr, nullptr);

	/*uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::cout << extensionCount << " extensions supported" << std::endl;

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	std::string warn;

	std::string filename = "Data/sponza_full/sponza.obj";
	std::string basepath = "Data/sponza_full/";
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), basepath.c_str()))
	{
		throw std::runtime_error(err);
	}*/

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (!Application::Inst()->MainLoop())
			break;
	}

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}