#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "Application/Application.h"

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(Application::Inst()->GetWidth(), Application::Inst()->GetHeight(), "Vulkan Clustered Forward", nullptr, nullptr);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (!Application::Inst()->MainLoop())
			break;
	}

	delete Application::Inst();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}