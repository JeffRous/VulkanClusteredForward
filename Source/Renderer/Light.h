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

	void SetAmbientIntensity(float i) { ambient_intensity = i; }
	float GetAmbientIntensity() { return ambient_intensity; }
	void SetDiffuseIntensity(float i) { diffuse_intensity = i; }
	float GetDiffuseIntensity() { return diffuse_intensity; }
	void SetSpecularIntensity(float i) { specular_intensity = i; }
	float GetSpecularIntensity() { return specular_intensity; }

protected:
	glm::vec3 color;
	float ambient_intensity;
	float diffuse_intensity;
	float specular_intensity;
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

	void SetAttenuationConstant(float a) { attenuation_constant = a; }
	float GetAttenuationConstant() { return attenuation_constant; }
	void SetAttenuationLinear(float a) { attenuation_linear = a; }
	float GetAttenuationLinear() { return attenuation_linear; }
	void SetAttenuationExp(float a) { attenuation_exp = a; }
	float GetAttenuationExp() { return attenuation_exp; }

protected:
	float radius;
	glm::vec3 position;
	float attenuation_constant;
	float attenuation_linear;
	float attenuation_exp;
};

#endif // !__LIGHT_H__
