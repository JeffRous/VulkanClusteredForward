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

	glm::mat4x4* GetViewMatrix() { return &matrix; }
	glm::mat4x4* GetProjectMatrix() { return &project_mat; }
	glm::mat4x4* GetViewProjectMatrix() { return &view_project_mtx; }

	glm::vec3 GetLookAtPosition() { return look_at; }

	void UpdateLookAt();
	glm::vec3 GetLookAtDir() { return look_at_dir; }
	float GetLookAtDist() { return look_at_dist; }
	glm::vec3 GetCurrentPosition() { return current_pos; }
	glm::vec3 GetCurrentLookAt() { return current_look_at; }
	glm::vec3 GetCurrentLookAtDir() { return current_look_at_dir; }
	float GetBaseScrollOffset() { return base_scroll_offset; }
	glm::vec2 GetBaseMoveOffset() { return base_move_offset; }

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

	glm::vec3 look_at_dir;
	float look_at_dist;
	glm::vec3 current_pos;
	glm::vec3 current_look_at;
	glm::vec3 current_look_at_dir;
	float base_scroll_offset;
	glm::vec2 base_move_offset;
};

#endif // !__CAMERA_H__
