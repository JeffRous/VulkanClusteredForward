#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class Model;
class Camera;
class Renderer
{
public:
	Renderer(GLFWwindow* win) : window(win) { camera = NULL; }
	virtual ~Renderer() {}

	virtual void RenderBegin() = 0;
	virtual void RenderEnd() = 0;
	virtual void Flush() = 0;
	virtual void WaitIdle() = 0;

	void SetCamera(Camera* c) { camera = c; }
	Camera* GetCamera() { return camera; }

protected:
	Camera* camera;
	GLFWwindow* window;
};

#endif // !__RENDERER_H__
