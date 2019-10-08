#ifndef __APPLICATION_H__
#define	__APPLICATION_H__

#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

	int GetControlState() { return control_state; }
	void SetControlState(int state) { control_state = state; }
	float GetScrollOffset() { return scroll_offset; }
	void SetScrollOffset(float offset) { scroll_offset = offset; }
	glm::vec2 GetMoveOffset() { return move_offset; }
	void SetMoveOffset(glm::vec2 offset) { move_offset = offset; }

private:
	void SceneUpdate(float dt);
	void SceneRender();

	void showFPS(GLFWwindow *pWindow);

private:
	Scene* current_scene;
	bool update_scene;

	Scene* next_scene;

	float delta_time;
	uint64_t last_time;

	double last_fps_time;
	int nb_frames;

	Renderer* renderer;

	GLFWwindow* current_window;

	int control_state;	// 0 normal 1 rotate 2 shift
	float scroll_offset;
	glm::vec2 move_offset;
};

#endif // !__APPLICATION_H__
