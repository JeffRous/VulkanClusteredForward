#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "TransformEntity.h"

class Camera : public TransformEntity
{
public:
	Camera(float s_width, float s_height);
	virtual ~Camera();

	void LookAt(glm::vec3& p);
	void SetFov(float f);
	void SetNearDistance(float n);
	void SetFarDistance(float f);

	virtual glm::mat4x4* UpdateMatrix();
	glm::mat4x4* UpdateProjectMatrix();

private:
	float fov;
	float near;
	float far;
	glm::vec3 look_at;

	float screen_width;
	float screen_height;

	bool project_changed;
	glm::mat4x4 project_mat;
};

#endif // !__CAMERA_H__
