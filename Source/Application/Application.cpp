#include <time.h>
#include <stdio.h>
#include <string>

#include "Application.h"
#include "Scene/Scene.h"
#include "Renderer/VRenderer.h"
#include "Renderer/Camera.h"
#include "Common/Utils.h"

#define SCREEN_WIDTH	1280.0f
#define SCREEN_HEIGHT	720.0f

Application* Application::inst = NULL;

Application::Application()
{
	current_scene = NULL;
	next_scene = NULL;
	update_scene = false;
	delta_time = 0.0f;
	control_state = 0;
	scroll_offset = 0.0f;
	key_pressed = false;
	last_time = Utils::GetMS();
}

Application::~Application()
{
	glfwSetInputMode(current_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (renderer != NULL)
	{
		renderer->WaitIdle();
	}
	if (update_scene)
	{
		current_scene->OnExit();
	}
	if (current_scene)
	{
		delete current_scene;
	}
	if (renderer != NULL)
	{
		delete renderer;
	}
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Application::Inst()->SetMoveOffset(glm::vec2(xpos, ypos));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	int controlState = Application::Inst()->GetControlState();
	if (button == GLFW_MOUSE_BUTTON_RIGHT)	/// shift look at
	{
		if (action == GLFW_PRESS && controlState == 0)
		{
			controlState = 2;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else if (action == GLFW_RELEASE && controlState == 2)
		{
			controlState = 0;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT)	/// rotate around look at
	{
		if (action == GLFW_PRESS && controlState == 0)
		{
			controlState = 1;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else if (action == GLFW_RELEASE && controlState == 1)
		{
			controlState = 0;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	Application::Inst()->SetControlState(controlState);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	float scrollOffset = Application::Inst()->GetScrollOffset();
	scrollOffset += (float)yoffset;
	Application::Inst()->SetScrollOffset(scrollOffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_RELEASE)
	{
		Application::Inst()->SetKeyPressed(key);
	}
}

void Application::CreateRenderer(GLFWwindow* window)
{
	current_window = window;
	VulkanRenderer* vRenderer = new VulkanRenderer(window);
	renderer = vRenderer;

	vRenderer->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
	std::string tex_path = "Data/shader/default.png";
	vRenderer->SetDefaultTex(tex_path);

	/// input initialize
	glfwSetInputMode(current_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(current_window, mouse_button_callback);
	glfwSetScrollCallback(current_window, scroll_callback);
	glfwSetKeyCallback(current_window, key_callback);
}

float Application::GetWidth()
{
	return SCREEN_WIDTH;
}

float Application::GetHeight()
{
	return SCREEN_HEIGHT;
}

void Application::SetRendererCamera(Camera* cam)
{
	renderer->SetCamera(cam);
}

void Application::showFPS(GLFWwindow *pWindow)
{
	// Measure speed
	double currentTime = glfwGetTime();
	double delta = currentTime - last_fps_time;
	nb_frames++;
	if (delta >= 1.0) { // If last cout was more than 1 sec ago
		double fps = double(nb_frames) / delta;
		const char* mode = "No Cluste Shading";
		if (((VulkanRenderer*)renderer)->IsClusteShading())
		{
			if (!((VulkanRenderer*)renderer)->IsCpuClusteCull())
				mode = "Computer Shader";
			else
			{
				if (!((VulkanRenderer*)renderer)->IsISPC())
				{
					mode = "Raw C++";
				}
				else
				{
					mode = "ISPC";
				}
			}
		}

		char title[256];
		title[255] = '\0';
		snprintf(title, 255, "[FPS: %3.2f] [ClusteShading: %s] [%s]", fps, ((VulkanRenderer*)renderer)->IsClusteShading() ? "ON" : "OFF", mode);
		glfwSetWindowTitle(pWindow, title);
		nb_frames = 0;
		last_fps_time = currentTime;
	}
}

bool Application::MainLoop()
{
	/// time offset
	uint64_t nowTime = Utils::GetMS();
	delta_time = (float)(nowTime - last_time) / 1000.0f;
	last_time = nowTime;

	/// show fps
	showFPS(current_window);

	/// logic
	SceneUpdate(delta_time);

	/// flush
	renderer->Flush();

	key_pressed = false;
	return true;
}

void Application::NextScene(Scene* scene)
{
	if (current_scene == NULL)
	{
		current_scene = scene;
	}
	else
	{
		next_scene = scene;
	}
}

void Application::SceneUpdate(float dt)
{
	bool scene_updated = false;

	do {
		if (current_scene == NULL)
			return;

		if (!update_scene && !current_scene->OnEnter())
		{
			current_scene->OnExit();
			renderer->OnSceneExit();
		}
		else
		{
			update_scene = current_scene->OnUpdate(dt);
			scene_updated = true;
			if (!update_scene || next_scene != NULL)
			{
				current_scene->OnExit();
				renderer->OnSceneExit();
				update_scene = false;
			}
		}

		if (!update_scene)
		{
			if (current_scene != NULL)
			{
				delete current_scene;
			}
			current_scene = next_scene;
			next_scene = NULL;
		}
	} while (!scene_updated);
}

void Application::SceneRender()
{
	renderer->RenderBegin();

	if (update_scene)
	{
		current_scene->OnRender(renderer);
	}

	renderer->RenderEnd();
}