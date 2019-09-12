#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "Application/Application.h"
#include "Renderer/VRenderer.h"
#include "Material.h"

Material::Material()
{
	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();
	vRenderer->AllocateDescriptorSets(desc_sets);
	desc_sets_updated = false;
}

Material::~Material()
{
	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();
	vRenderer->FreeDescriptorSets(desc_sets);

	if (ambient_tex != NULL)
		delete ambient_tex;
	if (diffuse_tex != NULL)
		delete diffuse_tex;
	if (specular_tex != NULL)
		delete specular_tex;
	if (specular_highlight_tex != NULL)
		delete specular_highlight_tex;
	if (bump_tex != NULL)
		delete bump_tex;
	if (displacement_tex != NULL)
		delete displacement_tex;
	if (alpha_tex != NULL)
		delete alpha_tex;
	if (reflection_tex != NULL)
		delete reflection_tex;
}

void Material::InitWithTinyMat(tinyobj::material_t* mat, std::string& basePath)
{
	tiny_mat = mat;

	/// load textures
	std::string fullPath;
	if (mat->ambient_texname != "")
	{
		fullPath = basePath + "/" + mat->ambient_texname;
		ambient_tex = new Texture(fullPath);
	}
	if (mat->diffuse_texname != "")
	{
		fullPath = basePath + "/" + mat->diffuse_texname;
		diffuse_tex = new Texture(fullPath);
	}
	if (mat->specular_texname != "")
	{
		fullPath = basePath + "/" + mat->specular_texname;
		specular_tex = new Texture(fullPath);
	}
	if (mat->specular_highlight_texname != "")
	{
		fullPath = basePath + "/" + mat->specular_highlight_texname;
		specular_highlight_tex = new Texture(fullPath);
	}
	if (mat->bump_texname != "" || mat->normal_texname != "")	// cryengine use bump as normal texture
	{
		if(mat->normal_texname != "")
			fullPath = basePath + "/" + mat->normal_texname;
		if (mat->bump_texname != "")
			fullPath = basePath + "/" + mat->bump_texname;
		bump_tex = new Texture(fullPath);
	}
	if (mat->displacement_texname != "")
	{
		fullPath = basePath + "/" + mat->displacement_texname;
		displacement_tex = new Texture(fullPath);
	}
	if (mat->alpha_texname != "")
	{
		fullPath = basePath + "/" + mat->alpha_texname;
		alpha_tex = new Texture(fullPath);
	}
	if (mat->reflection_texname != "")
	{
		fullPath = basePath + "/" + mat->reflection_texname;
		reflection_tex = new Texture(fullPath);
	}
}