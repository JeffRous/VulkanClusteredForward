#include <windows.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "Application/Application.h"
#include "VRenderer.h"
#include "Camera.h"

Camera::Camera(float s_width, float s_height)
	:screen_width(s_width)
	,screen_height(s_height)
{
	glm::vec3 p = glm::vec3(0,0,-5);
	SetPosition(p);
	p = glm::vec3(0);
	LookAt(p);
	SetNearDistance(0.1f);
	SetFarDistance(100.0f);
	SetFov(90.0f);
}

Camera::~Camera()
{
}

void Camera::LookAt(glm::vec3& p)
{
	transform_changed = true;
	look_at = p;
}

void Camera::SetFov(float f)	/// degree
{
	fov = f;
	project_changed = true;
}

void Camera::SetNearDistance(float n)
{
	near_clamp = n;
	project_changed = true;
}

void Camera::SetFarDistance(float f)
{
	far_clamp = f;
	project_changed = true;
}

glm::mat4x4* Camera::UpdateMatrix()
{
	if (transform_changed)
	{
		matrix = glm::lookAt(position, look_at, glm::vec3(0, -1, 0));	/// vulkan is right-hand and y is downward
		transform_changed = false;
	}
	return &matrix;
}

glm::mat4x4* Camera::UpdateProjectMatrix()
{
	if (project_changed)
	{
		project_mat = glm::perspective(glm::radians(fov), screen_width/screen_height, near_clamp, far_clamp); /// vulkan is right-hand
		project_changed = false;
	}
	return &project_mat;
}

void Camera::UpdateViewProject()
{
	glm::mat4x4* viewMtx = UpdateMatrix();
	glm::mat4x4* projMtx = UpdateProjectMatrix();
	glm::mat4x4 clipMtx = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	view_project_mtx = clipMtx * (*projMtx) * (*viewMtx);
}