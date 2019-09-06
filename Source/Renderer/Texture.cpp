#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "Application/Application.h"
#include "Renderer/VREnderer.h"
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>	/// implement

TextureData::TextureData(std::string& path)
{
	ref_count = 0;
	LoadFromPath(path);

	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();
	uint32_t texSize = GetWidth() * GetHeight() * 4;
	vRenderer->CreateImageBuffer(pixels, texSize, buffer, mem);

	if (!SaveOriginalPixel)
	{
		if (pixels != NULL)
		{
			stbi_image_free(pixels);
			pixels = NULL;
		}
	}

	vRenderer->CreateImage(GetWidth(), GetHeight(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture_image, texture_image_memory);
}

TextureData::~TextureData()
{
	assert(ref_count == 0);

	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();
	vRenderer->CleanImage(texture_image, texture_image_memory);
	vRenderer->CleanBuffer(buffer, mem);

	if (pixels != NULL)
	{
		stbi_image_free(pixels);
	}
}

bool TextureData::LoadFromPath(std::string& path)
{
	pixels = stbi_load(path.c_str(), &tex_width, &tex_height, &tex_channel, STBI_rgb_alpha);
	if (pixels == NULL)
		return false;

	tex_path = path;
	return true;
}

std::map<std::string, TextureData*> Texture::texture_cache;

Texture::Texture(std::string& path)
{
	InitWithPath(path);
}

Texture::~Texture()
{
	InitWithTextureData(NULL);
}

bool Texture::InitWithPath(std::string& path)
{
	std::map<std::string, TextureData*>::iterator iter = texture_cache.find(path);
	if (iter != texture_cache.end())
	{
		InitWithTextureData(iter->second);
		return true;
	}

	TextureData* texData = new TextureData(path);
	InitWithTextureData(texData);
	texture_cache.insert(std::make_pair(path, texData));
	return true;
}

void Texture::InitWithTextureData(TextureData* texData)
{
	if (tex_data != NULL)
	{
		tex_data->DelRefCount();
		if (tex_data->RefCount() == 0)
		{
			if (tex_data->Path() != "")	/// have path then remove from cache
			{
				texture_cache.erase(tex_data->Path());
			}
			delete tex_data;
		}
	}
	if (texData != NULL)
	{
		texData->AddRefCount();
	}
	tex_data = texData;
}