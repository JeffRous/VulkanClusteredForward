#ifndef __LIGHT_H__
#define	__LIGHT_H__

#include <string>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class Light
{
public:
	Light();
	virtual ~Light();

	void SetColor(const glm::vec3& c) { color = c; }
	glm::vec3& GetColor() { return color; }
	void SetIntensity(const glm::vec3& i) { intensity = i; }
	glm::vec3& GetIntensity() { return intensity; }

protected:
	glm::vec3 color;
	glm::vec3 intensity;
};

class PointLight : public Light
{
public:
	PointLight();
	virtual ~PointLight();

	void SetRadius(float r) { radius = r; }
	float GetRadius() { return radius; }
	void SetPosition(const glm::vec3& p) { position = p; }
	glm::vec3& GetPosition() { return position; }

protected:
	float radius;
	glm::vec3 position;
};

#endif // !__LIGHT_H__
