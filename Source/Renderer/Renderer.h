#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <iostream>

class Model;
class Camera;
class Renderer
{
public:
	Renderer(GLFWwindow* win) : window(win) { camera = NULL; }
	virtual ~Renderer() {}

	virtual void DrawModel(Model* model) = 0;

	virtual void RenderBegin() = 0;
	virtual void RenderEnd() = 0;
	virtual void Flush() = 0;

	virtual void WaitIdle() = 0;

	void SetCamera(Camera* c) { camera = c; }

protected:
	Camera* camera;
	GLFWwindow* window;
};

#endif // !__RENDERER_H__
