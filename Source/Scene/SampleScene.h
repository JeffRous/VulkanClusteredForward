#ifndef __SAMPLE_SCENE_H__
#define __SAMPLE_SCENE_H__

#include "Scene.h"

class Camera;
class TOModel;
class SampleScene : public Scene
{
public:
	SampleScene();
	virtual ~SampleScene();

	virtual bool OnEnter();
	virtual bool OnUpdate(float dt);
	virtual void OnExit();

	virtual void OnRender(Renderer* render);

private:
	Camera* camera;
	float colorR;

	TOModel* model;
};

#endif // !__SAMPLE_SCENE_H__
