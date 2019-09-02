#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "TransformEntity.h"

class VulkanRenderer;
class Camera : public TransformEntity
{
	friend class VulkanRenderer;
public:
	Camera(float s_width, float s_height);
	virtual ~Camera();

	void LookAt(glm::vec3& p);
	void SetFov(float f);	// degree
	void SetNearDistance(float n);
	void SetFarDistance(float f);

	virtual glm::mat4x4* UpdateMatrix();

	glm::mat4x4* GetViewProjectMatrix() { return &view_project_mtx; }

private:
	void UpdateViewProject();
	glm::mat4x4* UpdateProjectMatrix();

private:
	float fov;
	float near_clamp;
	float far_clamp;
	glm::vec3 look_at;

	float screen_width;
	float screen_height;

	bool project_changed;
	glm::mat4x4 project_mat;

	glm::mat4x4 view_project_mtx;
};

#endif // !__CAMERA_H__
