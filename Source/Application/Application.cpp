#include <time.h>

#include "Application.h"
#include "Scene/Scene.h"
#include "Renderer/Renderer.h"

#define SCREEN_WIDTH	1920.0f
#define SCREEN_HEIGHT	1080.0f

Application* Application::inst = NULL;

Application::Application()
{
	current_scene = NULL;
	next_scene = NULL;
	update_scene = false;
	change_scene = false;
	delta_time = 0.0f;
	time(&last_time);
}

Application::~Application()
{
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
	time_t nowTime;
	time(&nowTime);
	delta_time = (float)difftime(nowTime, last_time);
	last_time = nowTime;

	/// logic
	SceneUpdate(delta_time);

	/// render
	renderer->Flush();

	return true;
}

void Application::NextScene(Scene* scene)
{
	next_scene = scene;
	change_scene = true;
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
			if (!update_scene)
			{
				current_scene->OnExit();
			}
		}

		if (change_scene)
		{
			if (current_scene != NULL)
			{
				delete current_scene;
			}
			current_scene = next_scene;
			next_scene = NULL;
			change_scene = false;
		}
	} while (!scene_updated);
}