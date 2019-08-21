#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <exception>
#include <conio.h>

#include "Application/Application.h"
#include "Scene/SampleScene.h"

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	GLFWwindow* window = glfwCreateWindow((int)Application::Inst()->GetWidth(), (int)Application::Inst()->GetHeight(), "Vulkan Clustered Forward", nullptr, nullptr);
	try {
		Application::Inst()->CreateRenderer(window);
		Application::Inst()->NextScene(new SampleScene());
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			if (!Application::Inst()->MainLoop())
				break;
		}
	}
	catch (std::exception& e)
	{
		std::cout << "Catch exception: " << e.what() << std::endl;
		std::cout << "Press any key to exit." << std::endl;
		while (!_getch()) {}
	}
	delete Application::Inst();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}