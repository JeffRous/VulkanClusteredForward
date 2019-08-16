#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <iostream>

class Model;
class Camera;
class Renderer
{
public:
	Renderer() { camera = NULL; }
	virtual ~Renderer() {}

	virtual void DrawModel(Model* model) = 0;

	virtual void Flush() = 0;

	void SetCamera(Camera* c) { camera = c; }

protected:
	Camera* camera;
};

#endif // !__RENDERER_H__
