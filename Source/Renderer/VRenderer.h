﻿#ifndef __VULKAN_RENDERER_H__
#define	__VULKAN_RENDERER_H__

#include <vector>
#include <set>
#include <array>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Renderer.h"

#define MAX_LIGHT_NUM 16

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> computeFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
	}
};

/// one buffers(simple first)
struct Vertex {
	glm::vec4 pos;
	glm::vec3 color;
	glm::vec3 texcoord;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

/// transform data for shader
struct TransformData {
	glm::mat4x4 mvp;
	glm::mat4x4 model;
	glm::mat4x4 view;
	glm::mat4x4 proj;
	glm::mat4x4 proj_view;
	glm::vec3 cam_pos;
};

/// material flag for shader
struct MaterialData {
	int has_albedo_map;
	int has_normal_map;
};

/// light structure for shader
struct PointLightData {
	glm::vec3 pos;
	float radius;
	glm::vec3 color;
	float ambient_intensity;
	float diffuse_intensity;
	float specular_intensity;
	float attenuation_constant;
	float attenuation_linear;
	float attenuation_exp;
};

class Texture;
class Material;
class PointLight;
class VulkanRenderer : public Renderer
{
	const int MAX_MATERIAL_NUM = 50;
public:
	VulkanRenderer(GLFWwindow* win);
	virtual ~VulkanRenderer();

	virtual void RenderBegin();
	virtual void RenderEnd();
	virtual void Flush();
	virtual void WaitIdle();

	virtual void OnSceneExit();

	inline void SetClearColor(VkClearValue c) { clear_color = c; }
	void SetDefaultTex(std::string& path);
	inline VkCommandBuffer CurrentCommandBuffer() { return command_buffers[active_command_buffer_idx]; }

	void CreateVertexBuffer( void* vdata, uint32_t single, uint32_t length, VkBuffer& buffer, VkDeviceMemory& mem);
	void CreateIndexBuffer(void* idata, uint32_t single, uint32_t length, VkBuffer& buffer, VkDeviceMemory& mem);
	void CreateImageBuffer(void* imageData, uint32_t length, VkBuffer& buffer, VkDeviceMemory& mem);
	void CreateComputeBuffer(void* computeData, uint32_t length, VkBuffer& buffer, VkDeviceMemory& mem);
	void CreateUniformBuffer(void** data, uint32_t length, VkBuffer& buffer, VkDeviceMemory& mem);
	void UnmapBufferMemory(VkDeviceMemory& mem);
	void CleanBuffer(VkBuffer& buffer, VkDeviceMemory& mem);

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void CleanImage(VkImage& image, VkDeviceMemory& imageMem, VkImageView& imageView);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void SetMvpMatrix(glm::mat4x4& mvpMtx);
	void SetModelMatrix(glm::mat4x4& mtx);
	void SetViewMatrix(glm::mat4x4& mtx);
	void SetProjMatrix(glm::mat4x4& mtx);
	void SetProjViewMatrix(glm::mat4x4& mtx);
	void SetCamPos(glm::vec3& pos);
	void SetTexture(Texture* tex);
	void SetNormalTexture(Texture* tex);

	void AllocateDescriptorSets(VkDescriptorSet* descSets);
	void UpdateMaterial(Material* mat);
	void FreeDescriptorSets(VkDescriptorSet* descSets);

	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	void CreateTextureSampler(VkSampler* sampler);
	void DestroyTextureSampler(VkSampler* sampler);

	void AddLight(PointLight* light);
	void ClearLight();

	/*
		Next week
		Change uniform buffer to hold more matrix	(update uniform data individually)
		Add uniform buffer to hold light
		Add uniform buffer for material
		Add a normal texture sampler
		Fix the code for forward rendering
	*/

private:
	std::array<VkVertexInputBindingDescription, 1> GetBindingDescription();
	std::array<VkVertexInputAttributeDescription, 6> GetAttributeDescriptions();

	void CreateInstance();
	bool CheckValidationLayerSupport();

	void CreateSurface();

	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	void CreateLogicDevice();

	void CreateSwapChain();
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void CreateImageViews();

	void CreateRenderPass();

	void CreateGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);

	void CreateComputeClustePipeline();
	void CreateCullClustePipeline();

	void CreateDepthResources();
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat FindDepthFormat();
	bool HasStencilComponent(VkFormat format);

	void CreateFramebuffers();

	void CreateCommandPool();
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void CreateCommandBuffers();

	void CreateUniformBuffers();

	void CreateDescriptorSetsPool();

	void CreateSemaphores();

	void CleanUp();

private:
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkQueue graphics_queue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swap_chain;
	std::vector<VkImage> swap_chain_images;
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;
	std::vector<VkImageView> swap_chain_image_views;
	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;
	VkRenderPass render_pass;
	VkDescriptorSetLayout desc_layout;
	VkDescriptorPool desc_pool;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	VkDescriptorSetLayout comp_cluste_desc_layout;
	VkPipelineLayout comp_cluste_pipeline_layout;
	VkPipeline comp_cluste_pipeline;
	VkDescriptorSetLayout cull_cluste_desc_layout;
	VkPipelineLayout cull_cluste_pipeline_layout;
	VkPipeline cull_cluste_pipeline;
	std::vector<VkFramebuffer> swap_chain_framebuffers;
	VkCommandPool command_pool;
	std::vector<VkCommandBuffer> command_buffers;
	VkSemaphore image_available_semaphore;
	VkSemaphore render_finished_semaphore;
	VkFence in_flight_fence;
	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	VkClearValue clear_color;
	uint32_t active_command_buffer_idx;
	uint32_t last_command_buffer_idx;

	VkDescriptorImageInfo* image_info;
	VkDescriptorImageInfo* normal_image_info;
	Texture* default_tex;

	std::vector<PointLightData> light_infos;
	
	/// uniform buffers
	VkBuffer transform_uniform_buffer;
	VkDeviceMemory transform_uniform_buffer_memory;
	VkDescriptorBufferInfo transform_uniform_buffer_info;
	void* transform_uniform_buffer_data;

	std::vector<VkBuffer> light_uniform_buffers;
	std::vector<VkDeviceMemory> light_uniform_buffer_memorys;
	std::vector<VkDescriptorBufferInfo> light_uniform_buffer_infos;
	std::vector<void*> light_uniform_buffer_datas;
};


#endif // !__VULKAN_RENDERER_H__
