#include "Camera.h"

Camera::Camera(float s_width, float s_height)
	:screen_width(s_width)
	,screen_height(s_height)
{
	glm::vec3 p = glm::vec3(0);
	LookAt(p);
}

Camera::~Camera()
{
	
}

void Camera::LookAt(glm::vec3& p)
{
	transform_changed = true;
	look_at = p;
}

void Camera::SetFov(float f)
{
	fov = f;
	project_changed = true;
}

void Camera::SetNearDistance(float n)
{
	near = n;
	project_changed = true;
}

void Camera::SetFarDistance(float f)
{
	far = f;
	project_changed = true;
}

glm::mat4x4* Camera::UpdateMatrix()
{
	if (transform_changed)
	{
		matrix = glm::lookAtRH(look_at, position, glm::vec3(0, 1, 0));	/// vulkan is right-hand
		transform_changed = false;
	}
	return &matrix;
}

glm::mat4x4* Camera::UpdateProjectMatrix()
{
	if (project_changed)
	{
		project_mat = glm::perspectiveRH(fov, screen_width/screen_height, near, far); /// vulkan is right-hand
		project_changed = false;
	}
	return &project_mat;
}