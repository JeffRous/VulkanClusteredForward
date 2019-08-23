#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "Renderer/VRenderer.h"
#include "Application/Application.h"
#include "TOModel.h"

const std::vector<Vertex> vertices = {
	{{0.0f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},

	{{0.0f, 0.5f, 0.51f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.51f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, -0.5f, 0.51f}, {0.0f, 0.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 3, 5, 4
};

TOModel::TOModel()
{
	
}

TOModel::~TOModel()
{
	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();

	vRenderer->CleanBuffer(index_buffer, index_buffer_memory);
	vRenderer->CleanBuffer(vertex_buffer, vertex_buffer_memory);
}

bool TOModel::LoadTestData()
{
	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();

	vRenderer->CreateVertexBuffer((void*)vertices.data(), sizeof(vertices[0]), vertices.size(), vertex_buffer, vertex_buffer_memory);
	vRenderer->CreateIndexBuffer((void*)indices.data(), sizeof(indices[0]), indices.size(), index_buffer, index_buffer_memory);

	return true;
}

void TOModel::Draw()
{
	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();
	VkCommandBuffer cb = vRenderer->CurrentCommandBuffer();

	VkBuffer vertexBuffers[] = { vertex_buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(cb, index_buffer, 0, VK_INDEX_TYPE_UINT16);
	vkCmdDrawIndexed(cb, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

bool TOModel::LoadFromPath(std::string path)
{
	std::string err;
	std::string warn;

	///std::string filename = "Data/sponza_full/sponza.obj";
	std::string basePath = "";
	int slashIdx = path.find_last_of("/");
	if (slashIdx >= 0)
	{
		basePath = path.substr(slashIdx, path.length() - slashIdx + 1);
	}

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), basePath.c_str()))
	{
		throw std::runtime_error(err);
		return false;
	}

	return true;
}