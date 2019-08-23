#ifndef __APPLICATION_H__
#define	__APPLICATION_H__

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

class Scene;
class Renderer;
class Camera;
class Application
{
	static Application* inst;

	Application();

public:
	static Application* Inst()
	{
		if (inst == NULL)
		{
			inst = new Application();
		}
		return inst;
	}

	virtual ~Application();
	void CreateRenderer(GLFWwindow* window);

	bool MainLoop();
	void NextScene(Scene* scene);

	float GetWidth();
	float GetHeight();

	void SetRendererCamera(Camera* cam);
	Renderer* GetRenderer() { return renderer; }

private:
	void SceneUpdate(float dt);
	void SceneRender();

private:
	Scene* current_scene;
	bool update_scene;

	Scene* next_scene;

	float delta_time;
	uint64_t last_time;

	Renderer* renderer;
};

#endif // !__APPLICATION_H__
