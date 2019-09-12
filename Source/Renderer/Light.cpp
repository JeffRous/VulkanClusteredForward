#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "Light.h"

Light::Light()
{
}

Light::~Light()
{
}

PointLight::PointLight()
{
}

PointLight::~PointLight()
{
}