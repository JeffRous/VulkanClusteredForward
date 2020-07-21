#ifndef __SAMPLE_SCENE_H__
#define __SAMPLE_SCENE_H__

#include "Scene.h"

class Camera;
class TOModel;
class PointLight;
class SampleScene : public Scene
{
	enum ShadingMode
	{
		NoClusteShading,
		ClusteShading_Compute,
		ClusteShading_RawCpu,
		ClusteShading_ISPC,
	};
public:
	SampleScene();
	virtual ~SampleScene();

	virtual bool OnEnter();
	virtual bool OnUpdate(float dt);
	virtual void OnExit();

	virtual void OnRender(Renderer* render);

private:
	void UpdateCameraByInput();

private:
	Camera* camera;
	float colorR;

	TOModel* model;
	int last_control_state;

	PointLight* light[MAX_LIGHT_NUM];

	ShadingMode shadingMode;
};

#endif // !__SAMPLE_SCENE_H__
