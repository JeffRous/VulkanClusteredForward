#ifndef __SAMPLE_SCENE_H__
#define __SAMPLE_SCENE_H__

#include "Scene.h"
#include "Renderer/Camera.h"

class SampleScene : public Scene
{
public:
	SampleScene();
	virtual ~SampleScene();

	virtual bool OnEnter();
	virtual bool OnUpdate(float dt);
	virtual void OnExit();

private:
	Camera* camera;

};

#endif // !__SAMPLE_SCENE_H__
