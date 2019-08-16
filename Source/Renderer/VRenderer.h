#ifndef __VULKAN_RENDERER_H__
#define	__VULKAN_RENDERER_H__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>

#include "Renderer.h"

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
	}
};

class VulkanRenderer : public Renderer
{
public:
	VulkanRenderer();
	virtual ~VulkanRenderer();

	virtual void DrawModel(Model* model);

	virtual void Flush();

private:
	void CreateInstance();
	bool CheckValidationLayerSupport();

	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	void CreateLogicDevice();

	void CleanUp();

private:
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkQueue graphics_queue;
};


#endif // !__VULKAN_RENDERER_H__
