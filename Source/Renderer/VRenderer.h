#ifndef __VULKAN_RENDERER_H__
#define	__VULKAN_RENDERER_H__

#include "Renderer.h"

class VulkanRenderer : public Renderer
{
public:
	VulkanRenderer();
	virtual ~VulkanRenderer();

	virtual void DrawModel(Model* model);

	virtual void Flush();
};


#endif // !__VULKAN_RENDERER_H__
