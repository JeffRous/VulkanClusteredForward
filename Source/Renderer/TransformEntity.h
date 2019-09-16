#ifndef __TRANSFORM_ENTITY_H__
#define __TRANSFORM_ENTITY_H__

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class TransformEntity
{
public:
	TransformEntity() { transform_changed = true; scale = 1.0f; }
	virtual ~TransformEntity() {}

	void SetScale(float s) { scale = s; transform_changed = true; }
	void SetPosition(glm::vec3& p) { position = p; transform_changed = true; }
	void SetRotation(glm::vec3& r) { rotation = r; transform_changed = true; }

	glm::vec3& GetPosition() { return position; }
	glm::mat4x4& GetMatrix() { return matrix; }

	virtual glm::mat4x4* UpdateMatrix()
	{
		if (transform_changed)
		{
			matrix = glm::identity<glm::mat4x4>();
			matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
			matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
			matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
			matrix = glm::scale(matrix, glm::vec3(scale));
			matrix = glm::translate(matrix, position);
			transform_changed = false;
		}
		return &matrix;
	}

protected:
	glm::vec3 position;
	glm::vec3 rotation;	// degree
	float scale;

	bool transform_changed;
	glm::mat4x4 matrix;
};

#endif // !__TRANSFORM_ENTITY_H__
