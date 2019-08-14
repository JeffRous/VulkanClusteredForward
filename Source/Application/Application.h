#ifndef __APPLICATION_H__
#define	__APPLICATION_H__

#include <iostream>

class Scene;
class Renderer;
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

	bool MainLoop();
	void NextScene(Scene* scene);

private:
	void SceneUpdate(float dt);

private:
	Scene* current_scene;
	bool update_scene;

	Scene* next_scene;
	bool change_scene;

	float delta_time;
	time_t last_time;

	Renderer* renderer;
};

#endif // !__APPLICATION_H__
