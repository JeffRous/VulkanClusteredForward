#include <time.h>

#include "Application.h"
#include "Scene/Scene.h"
#include "Renderer/VRenderer.h"
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
	last_time = Utils::GetMS();
}

Application::~Application()
{
	if (renderer != NULL)
	{
		renderer->WaitIdle();
		delete renderer;
	}
}

void Application::CreateRenderer(GLFWwindow* window)
{
	VulkanRenderer* vRenderer = new VulkanRenderer(window);
	vRenderer->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });

	renderer = vRenderer;
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

bool Application::MainLoop()
{
	/// time offset
	uint64_t nowTime = Utils::GetMS();
	delta_time = (float)(nowTime - last_time) / 1000.0f;
	last_time = nowTime;

	/// logic
	SceneUpdate(delta_time);

	/// render
	SceneRender();

	/// flush
	renderer->Flush();

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
		}
		else
		{
			update_scene = current_scene->OnUpdate(dt);
			scene_updated = true;
			if (!update_scene || next_scene != NULL)
			{
				current_scene->OnExit();
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