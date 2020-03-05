#include <algorithm>
#include <windows.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "Application/Application.h"
#include "Common/Utils.h"
#include "Camera.h"
#include "Texture.h"
#include "Material.h"
#include "Light.h"
#include "VRenderer.h"

#undef max
#undef min

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VulkanRenderer::VulkanRenderer(GLFWwindow* win)
	:Renderer(win)
{
	last_command_buffer_idx = UINT_MAX;
	CreateInstance();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateGraphicsPipeline();
	InitializeClusteRendering();
	CreateCommandPool();
	CreateDepthResources();
	CreateFramebuffers();
	CreateCommandBuffers();
	CreateUniformBuffers();
	CreateDescriptorSetsPool();
	CreateSemaphores();

	clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
	default_tex = NULL;
}

void VulkanRenderer::SetDefaultTex(std::string& path)
{
	default_tex = new Texture(path);
}

VulkanRenderer::~VulkanRenderer()
{
	if (default_tex)
	{
		delete default_tex;
	}
	CleanUp();
}

void VulkanRenderer::CreateInstance()
{
	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vlukan Clustered Forward";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanRenderer::UnmapBufferMemory(VkDeviceMemory& mem)
{
	vkUnmapMemory(device, mem);
}

void VulkanRenderer::CleanBuffer(VkBuffer& buffer, VkDeviceMemory& mem)
{
	vkDestroyBuffer(device, buffer, nullptr);
	vkFreeMemory(device, mem, nullptr);
}

void VulkanRenderer::CleanImage(VkImage& image, VkDeviceMemory& imageMem, VkImageView& imageView)
{
	vkDestroyImageView(device, imageView, nullptr);
	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, imageMem, nullptr);
}

void VulkanRenderer::CleanUp()
{
	for (int i = 0; i < light_uniform_buffers.size(); i++)
	{
		UnmapBufferMemory(light_uniform_buffer_memorys[i]);
		CleanBuffer(light_uniform_buffers[i], light_uniform_buffer_memorys[i]);
	}

	UnmapBufferMemory(transform_uniform_buffer_memory);
	CleanBuffer(transform_uniform_buffer, transform_uniform_buffer_memory);

	vkDestroyImageView(device, depth_image_view, nullptr);
	vkDestroyImage(device, depth_image, nullptr);
	vkFreeMemory(device, depth_image_memory, nullptr);

	vkDestroySemaphore(device, render_finished_semaphore, nullptr);
	vkDestroySemaphore(device, image_available_semaphore, nullptr);
	vkDestroyFence(device, in_flight_fence, nullptr);

	vkDestroyCommandPool(device, command_pool, nullptr);
	vkDestroyDescriptorSetLayout(device, desc_layout, nullptr);
	vkDestroyDescriptorPool(device, desc_pool, nullptr);

	for (auto framebuffer : swap_chain_framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	vkDestroyPipeline(device, graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
	vkDestroyRenderPass(device, render_pass, nullptr);

	ReleaseCompDescriptorSets();
	vkDestroyDescriptorPool(device, comp_desc_pool, nullptr);
	vkDestroyDescriptorSetLayout(device, comp_desc_layout, nullptr);
	vkDestroyPipelineLayout(device, comp_pipeline_layout, nullptr);
	vkDestroyCommandPool(device, comp_command_pool, nullptr);
	vkDestroyPipeline(device, comp_pipeline, nullptr);

	vkDestroyShaderModule(device, frag_shader_module, nullptr);
	vkDestroyShaderModule(device, vert_shader_module, nullptr);

	for (auto imageView : swap_chain_image_views) {
		vkDestroyImageView(device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(device, swap_chain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

bool VulkanRenderer::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

void VulkanRenderer::PickPhysicalDevice()
{
	physical_device = VK_NULL_HANDLE;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (IsDeviceSuitable(device)) {
			physical_device = device;
			break;
		}
	}

	if (physical_device == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool VulkanRenderer::IsDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);
	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) && indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices VulkanRenderer::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.computeFamily = i;	/// here we use one queue to deal with compute, no async compute here
			}
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

void VulkanRenderer::CreateSurface()
{
#if 0	/// for study, same meaning with below
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = glfwGetWin32Window(window);
	createInfo.hinstance = GetModuleHandle(nullptr);

	if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
#else
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
#endif
}

void VulkanRenderer::CreateLogicDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(physical_device);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.computeFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physical_device, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphics_queue);
	vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &comp_queue);
}

VkSurfaceFormatKHR VulkanRenderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { (uint32_t)Application::Inst()->GetWidth(), (uint32_t)Application::Inst()->GetHeight() };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void VulkanRenderer::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physical_device);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	/// post process use VK_IMAGE_USAGE_TRANSFER_DST_BIT 

	QueueFamilyIndices indices = FindQueueFamilies(physical_device);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swap_chain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swap_chain, &imageCount, nullptr);
	swap_chain_images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swap_chain, &imageCount, swap_chain_images.data());

	swap_chain_image_format = surfaceFormat.format;
	swap_chain_extent = extent;
}

void VulkanRenderer::CreateImageViews()
{
	swap_chain_image_views.resize(swap_chain_images.size());
	for (uint32_t i = 0; i < swap_chain_images.size(); i++) {
		swap_chain_image_views[i] = CreateImageView(swap_chain_images[i], swap_chain_image_format, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

VkImageView VulkanRenderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

void VulkanRenderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(device, image, imageMemory, 0);
}

void VulkanRenderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer =BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swap_chain_image_format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

std::array<VkVertexInputBindingDescription, 1> VulkanRenderer::GetBindingDescription()
{
	std::array<VkVertexInputBindingDescription, 1> bindingDescriptions = {};

	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}

std::array<VkVertexInputAttributeDescription, 6> VulkanRenderer::GetAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texcoord);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex, normal);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(Vertex, tangent);

	attributeDescriptions[5].binding = 0;
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[5].offset = offsetof(Vertex, bitangent);

	return attributeDescriptions;
}

void VulkanRenderer::CreateGraphicsPipeline()
{
	std::string vsCode;
	std::string psCode;
	vsCode = "Data/shader/tinyobj_vert.spv";
	psCode = "Data/shader/tinyobj_frag.spv";
	auto vertShaderCode = Utils::readFile(vsCode);
	auto fragShaderCode = Utils::readFile(psCode);

	vert_shader_module = createShaderModule(vertShaderCode);
	frag_shader_module = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vert_shader_module;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = frag_shader_module;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	/// fixed pipeline setting manually
	/// vertex buffer
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	auto bindingDescription = GetBindingDescription();
	auto attributeDescriptions = GetAttributeDescriptions();
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	/// primitive type
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	/// viewport
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swap_chain_extent.width;
	viewport.height = (float)swap_chain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	/// scissor
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swap_chain_extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	/// raster
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	/// multi-sample
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	/// depth / stencil
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional
	
	/// color blend
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	/// uniform layout
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBinding.pImmutableSamplers = NULL;

	VkDescriptorSetLayoutBinding layoutBinding1 = {};
	layoutBinding1.binding = 1;
	layoutBinding1.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding1.descriptorCount = 1;
	layoutBinding1.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBinding1.pImmutableSamplers = NULL;

	VkDescriptorSetLayoutBinding layoutBinding2 = {};
	layoutBinding2.binding = 2;
	layoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding2.descriptorCount = MAX_LIGHT_NUM;
	layoutBinding2.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBinding2.pImmutableSamplers = NULL;

	/// sampler layout
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 3;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding1 = {};
	samplerLayoutBinding1.binding = 4;
	samplerLayoutBinding1.descriptorCount = 1;
	samplerLayoutBinding1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding1.pImmutableSamplers = nullptr;
	samplerLayoutBinding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 5> bindings = { layoutBinding, layoutBinding1, layoutBinding2, samplerLayoutBinding, samplerLayoutBinding1 };
	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = NULL;
	descriptorLayout.bindingCount = static_cast<uint32_t>(bindings.size());
	descriptorLayout.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(device, &descriptorLayout, NULL, &desc_layout);

	/// pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &desc_layout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipeline_layout;
	pipelineInfo.renderPass = render_pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void VulkanRenderer::CreateCompPipeline()
{
	QueueFamilyIndices indices = FindQueueFamilies(physical_device);

	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[6] = {
		{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
		{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
		{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
		{3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
		{4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0},
		{5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, 0}
	};

	/// desc set for compute shader
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		0, 0, 6, descriptorSetLayoutBindings
	};
	vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &comp_desc_layout);

	/// pipeline layout
	VkPipelineLayoutCreateInfo computePipelineLayoutInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr, 0,
		1, &comp_desc_layout,
		0, nullptr
	};
	if (vkCreatePipelineLayout(device, &computePipelineLayoutInfo, nullptr, &comp_pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	/// pipeline
	VkComputePipelineCreateInfo computePipelineCreateInfos[2] = {
		{
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			0, 0,
			{
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				0, 0, VK_SHADER_STAGE_COMPUTE_BIT, createShaderModule(Utils::readFile("Data/shader/cluste_calc.spv")), "main", 0
			},
			comp_pipeline_layout, 0, 0
		},
		{
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			0, 0,
			{
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				0, 0, VK_SHADER_STAGE_COMPUTE_BIT, createShaderModule(Utils::readFile("Data/shader/cluste_culling.spv")), "main", 0
			},
			comp_pipeline_layout, 0, 0
		},
	};
	if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 2, computePipelineCreateInfos, nullptr, &comp_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create compute pipeline!");
	}

	// Separate command pool as queue family for compute may be different than graphics
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = indices.computeFamily.value();
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &comp_command_pool);

	// Command buffer
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = comp_command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device, &allocInfo, &comp_command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void VulkanRenderer::InitializeClusteRendering()
{
	CreateCompPipeline();

	/// create desc set pool for cluste shadering
	CreateCompDescriptorSetsPool();

	CreateCompDescriptorSets();
}

void VulkanRenderer::AllocateCompDescriptorSets(VkDescriptorSet* descSets)
{
	VkDescriptorSetLayout desc_layouts[3] = { comp_desc_layout, comp_desc_layout, comp_desc_layout };
	VkDescriptorSetAllocateInfo allocInfo[1];
	allocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo[0].pNext = NULL;
	allocInfo[0].descriptorPool = comp_desc_pool;
	allocInfo[0].descriptorSetCount = swap_chain_images.size();
	allocInfo[0].pSetLayouts = desc_layouts;
	vkAllocateDescriptorSets(device, allocInfo, descSets);
}

void VulkanRenderer::CreateCompDescriptorSets()
{
	/// set computer number and tile size in screen space
	tile_size.x = Application::Inst()->GetWidth() / 20;
	tile_size.y = Application::Inst()->GetHeight() / 20;
	group_num = glm::uvec3(20, 20, 10);

	/// allocate desc sets
	AllocateCompDescriptorSets(comp_desc_set);

	/// tile aabb
	VkDeviceSize bufferSize = sizeof(VolumeTileAABB) * 20 * 20 * 10;
	CreateComputeBuffer(&tile_aabbs_buffer_data, (uint32_t)bufferSize, tile_aabbs_buffer, tile_aabbs_buffer_memory);
	tile_aabbs_buffer_info.buffer = tile_aabbs_buffer;
	tile_aabbs_buffer_info.offset = 0;
	tile_aabbs_buffer_info.range = bufferSize;

	/// screen to view
	bufferSize = sizeof(ScreenToView);
	CreateUniformBuffer(&screen_to_view_buffer_data, (uint32_t)bufferSize, screen_to_view_buffer, screen_to_view_buffer_memory);
	ScreenToView* stv = (ScreenToView*)screen_to_view_buffer_data;
	stv->screenDimensions = glm::uvec2(Application::Inst()->GetWidth(), Application::Inst()->GetHeight());
	stv->tileSizes = glm::uvec4(tile_size, 0, 0);
	screen_to_view_buffer_info.buffer = screen_to_view_buffer;
	screen_to_view_buffer_info.offset = 0;
	screen_to_view_buffer_info.range = bufferSize;

	/// light datas
	bufferSize = sizeof(PointLightData) * MAX_LIGHT_NUM;
	CreateComputeBuffer(&light_datas_buffer_data, (uint32_t)bufferSize, light_datas_buffer, light_datas_buffer_memory);
	light_datas_buffer_info.buffer = light_datas_buffer;
	light_datas_buffer_info.offset = 0;
	light_datas_buffer_info.range = bufferSize;

	/// light indexes
	bufferSize = sizeof(glm::uint) * MAX_LIGHT_NUM * 20 * 20 * 10;
	CreateComputeBuffer(&light_indexes_buffer_data, (uint32_t)bufferSize, light_indexes_buffer, light_indexes_buffer_memory);
	light_indexes_buffer_info.buffer = light_indexes_buffer;
	light_indexes_buffer_info.offset = 0;
	light_indexes_buffer_info.range = bufferSize;

	/// light grids
	bufferSize = sizeof(glm::uint) * MAX_LIGHT_NUM * 20 * 20 * 10;
	CreateComputeBuffer(&light_grids_buffer_data, (uint32_t)bufferSize, light_grids_buffer, light_grids_buffer_memory);
	light_grids_buffer_info.buffer = light_grids_buffer;
	light_grids_buffer_info.offset = 0;
	light_grids_buffer_info.range = bufferSize;

	/// global index count
	bufferSize = sizeof(glm::uint);
	CreateComputeBuffer(&index_count_buffer_data, (uint32_t)bufferSize, index_count_buffer, index_count_buffer_memory);
	index_count_buffer_info.buffer = index_count_buffer;
	index_count_buffer_info.offset = 0;
	index_count_buffer_info.range = bufferSize;
}

void VulkanRenderer::ReleaseCompDescriptorSets()
{
	UnmapBufferMemory(tile_aabbs_buffer_memory);
	UnmapBufferMemory(screen_to_view_buffer_memory);
	UnmapBufferMemory(light_datas_buffer_memory);
	UnmapBufferMemory(light_indexes_buffer_memory);
	UnmapBufferMemory(light_grids_buffer_memory);
	UnmapBufferMemory(index_count_buffer_memory);
	CleanBuffer(tile_aabbs_buffer, tile_aabbs_buffer_memory);
	CleanBuffer(screen_to_view_buffer, screen_to_view_buffer_memory);
	CleanBuffer(light_datas_buffer, light_datas_buffer_memory);
	CleanBuffer(light_indexes_buffer, light_indexes_buffer_memory);
	CleanBuffer(light_grids_buffer, light_grids_buffer_memory);
	CleanBuffer(index_count_buffer, index_count_buffer_memory);
	FreeCompDescriptorSets(comp_desc_set);
}

void VulkanRenderer::UpdateComputeDescriptorSet()
{
	vkQueueWaitIdle(comp_queue);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(comp_command_buffer, &beginInfo);

	vkCmdBindPipeline(comp_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, comp_pipeline);

	/// set descriptor sets
	std::array<VkWriteDescriptorSet, 6> descriptorWrites = {};
	descriptorWrites[0] = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].pNext = NULL;
	descriptorWrites[0].dstSet = comp_desc_set[active_command_buffer_idx];
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites[0].pBufferInfo = &tile_aabbs_buffer_info;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].dstBinding = 0;

	descriptorWrites[1] = {};
	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].pNext = NULL;
	descriptorWrites[1].dstSet = comp_desc_set[active_command_buffer_idx];
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[1].pBufferInfo = &screen_to_view_buffer_info;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].dstBinding = 1;

	descriptorWrites[2] = {};
	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].pNext = NULL;
	descriptorWrites[2].dstSet = comp_desc_set[active_command_buffer_idx];
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites[2].pBufferInfo = &light_datas_buffer_info;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].dstBinding = 2;

	descriptorWrites[3] = {};
	descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[3].pNext = NULL;
	descriptorWrites[3].dstSet = comp_desc_set[active_command_buffer_idx];
	descriptorWrites[3].descriptorCount = 1;
	descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites[3].pBufferInfo = &light_indexes_buffer_info;
	descriptorWrites[3].dstArrayElement = 0;
	descriptorWrites[3].dstBinding = 3;

	descriptorWrites[4] = {};
	descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[4].pNext = NULL;
	descriptorWrites[4].dstSet = comp_desc_set[active_command_buffer_idx];
	descriptorWrites[4].descriptorCount = 1;
	descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites[4].pBufferInfo = &light_grids_buffer_info;
	descriptorWrites[4].dstArrayElement = 0;
	descriptorWrites[4].dstBinding = 4;

	descriptorWrites[5] = {};
	descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[5].pNext = NULL;
	descriptorWrites[5].dstSet = comp_desc_set[active_command_buffer_idx];
	descriptorWrites[5].descriptorCount = 1;
	descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites[5].pBufferInfo = &index_count_buffer_info;
	descriptorWrites[5].dstArrayElement = 0;
	descriptorWrites[5].dstBinding = 5;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, NULL);

	vkCmdBindDescriptorSets(comp_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, comp_pipeline_layout, 0, 1, &comp_desc_set[active_command_buffer_idx], 0, nullptr);

	vkCmdDispatch(comp_command_buffer, group_num.x, group_num.y, group_num.z);

	vkEndCommandBuffer(comp_command_buffer);
}

void VulkanRenderer::CreateDepthResources()
{
	VkFormat depthFormat = FindDepthFormat();
	CreateImage(swap_chain_extent.width, swap_chain_extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);
	depth_image_view = CreateImageView(depth_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	TransitionImageLayout(depth_image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat VulkanRenderer::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanRenderer::FindDepthFormat()
{
	return FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool VulkanRenderer::HasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanRenderer::CreateFramebuffers()
{
	swap_chain_framebuffers.resize(swap_chain_images.size());
	for (size_t i = 0; i < swap_chain_image_views.size(); i++) {
		std::array<VkImageView, 2> attachments = {
			swap_chain_image_views[i],
			depth_image_view
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = render_pass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swap_chain_extent.width;
		framebufferInfo.height = swap_chain_extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swap_chain_framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VulkanRenderer::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physical_device);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

VkCommandBuffer VulkanRenderer::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanRenderer::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphics_queue);

	vkFreeCommandBuffers(device, command_pool, 1, &commandBuffer);
}

void VulkanRenderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};
	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
	EndSingleTimeCommands(commandBuffer);
}

uint32_t VulkanRenderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanRenderer::CreateVertexBuffer(void* vdata, uint32_t single, uint32_t length, VkBuffer& buffer, VkDeviceMemory& mem)
{
	VkDeviceSize bufferSize = single * length;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, mem);

	void* data;
	vkMapMemory(device, mem, 0, bufferSize, 0, &data);
	memcpy(data, vdata, (size_t)bufferSize);
	vkUnmapMemory(device, mem);
}

void VulkanRenderer::CreateIndexBuffer(void* idata, uint32_t single, uint32_t length, VkBuffer& buffer, VkDeviceMemory& mem)
{
	VkDeviceSize bufferSize = single * length;

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, mem);

	void* data;
	vkMapMemory(device, mem, 0, bufferSize, 0, &data);
	memcpy(data, idata, (size_t)bufferSize);
	vkUnmapMemory(device, mem);
}

void VulkanRenderer::CreateImageBuffer(void* imageData, uint32_t length, VkBuffer& buffer, VkDeviceMemory& mem)
{
	VkDeviceSize bufferSize = length;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, mem);

	void* data;
	vkMapMemory(device, mem, 0, bufferSize, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, mem);
}

void VulkanRenderer::CreateComputeBuffer(void** data, uint32_t length, VkBuffer& buffer, VkDeviceMemory& mem)
{
	VkDeviceSize bufferSize = length;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, mem);

	vkMapMemory(device, mem, 0, bufferSize, 0, data);
}

void VulkanRenderer::CreateUniformBuffer(void** data, uint32_t length, VkBuffer& buffer, VkDeviceMemory& mem)
{
	VkDeviceSize bufferSize = length;
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, mem);

	vkMapMemory(device, mem, 0, bufferSize, 0, data);
}

void VulkanRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanRenderer::CreateCommandBuffers()
{
	command_buffers.resize(swap_chain_framebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)command_buffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void VulkanRenderer::UpdateMaterial(Material* mat)
{
	VkDescriptorSet* descSets = mat->GetDescriptorSets();
	if (!mat->IsDescSetUpdated())
	{
		std::array<VkWriteDescriptorSet, 5> descriptorWrites = {};
		descriptorWrites[0] = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].pNext = NULL;
		descriptorWrites[0].dstSet = descSets[active_command_buffer_idx];
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].pBufferInfo = &transform_uniform_buffer_info;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].dstBinding = 0;

		descriptorWrites[1] = {};
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].pNext = NULL;
		descriptorWrites[1].dstSet = descSets[active_command_buffer_idx];
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].pBufferInfo = mat->GetBufferInfo();
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].dstBinding = 1;

		descriptorWrites[2] = {};
		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].pNext = NULL;
		descriptorWrites[2].dstSet = descSets[active_command_buffer_idx];
		descriptorWrites[2].descriptorCount = light_uniform_buffer_infos.size();
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[2].pBufferInfo = light_uniform_buffer_infos.data();
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].dstBinding = 2;

		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = descSets[active_command_buffer_idx];
		descriptorWrites[3].dstBinding = 3;
		descriptorWrites[3].dstArrayElement = 0;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].pImageInfo = image_info;

		descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[4].dstSet = descSets[active_command_buffer_idx];
		descriptorWrites[4].dstBinding = 4;
		descriptorWrites[4].dstArrayElement = 0;
		descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[4].descriptorCount = 1;
		descriptorWrites[4].pImageInfo = normal_image_info;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, NULL);

		mat->SetDescUpdated();
	}

	vkCmdBindDescriptorSets(command_buffers[active_command_buffer_idx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descSets[active_command_buffer_idx], 0, nullptr);
}

void VulkanRenderer::SetMvpMatrix(glm::mat4x4& mvpMtx)
{
	TransformData* transData = (TransformData*)transform_uniform_buffer_data;
	memcpy(&transData->mvp, &mvpMtx, sizeof(glm::mat4x4));
}

void VulkanRenderer::SetModelMatrix(glm::mat4x4& mtx)
{
	TransformData* transData = (TransformData*)transform_uniform_buffer_data;
	memcpy(&transData->model, &mtx, sizeof(glm::mat4x4));
}

void VulkanRenderer::SetViewMatrix(glm::mat4x4& mtx)
{
	TransformData* transData = (TransformData*)transform_uniform_buffer_data;
	memcpy(&transData->view, &mtx, sizeof(glm::mat4x4));
}

void VulkanRenderer::SetProjMatrix(glm::mat4x4& mtx)
{
	TransformData* transData = (TransformData*)transform_uniform_buffer_data;
	memcpy(&transData->proj, &mtx, sizeof(glm::mat4x4));
}

void VulkanRenderer::SetProjViewMatrix(glm::mat4x4& mtx)
{
	TransformData* transData = (TransformData*)transform_uniform_buffer_data;
	memcpy(&transData->proj_view, &mtx, sizeof(glm::mat4x4));
}

void VulkanRenderer::SetCamPos(glm::vec3& pos)
{
	TransformData* transData = (TransformData*)transform_uniform_buffer_data;
	memcpy(&transData->cam_pos, &pos, sizeof(glm::vec3));
}

void VulkanRenderer::SetTexture(Texture* tex)
{
	if (tex != NULL)
		image_info = tex->GetImageInfo();
	else
		image_info = default_tex->GetImageInfo();
}

void VulkanRenderer::SetNormalTexture(Texture* tex)
{
	if (tex != NULL)
		normal_image_info = tex->GetImageInfo();
	else
		normal_image_info = default_tex->GetImageInfo();
}

void VulkanRenderer::CreateUniformBuffers()
{
	/// transform uniform buffer
	VkDeviceSize bufferSize = sizeof(TransformData);
	CreateUniformBuffer(&transform_uniform_buffer_data, (uint32_t)bufferSize, transform_uniform_buffer, transform_uniform_buffer_memory);
	transform_uniform_buffer_info.buffer = transform_uniform_buffer;
	transform_uniform_buffer_info.offset = 0;
	transform_uniform_buffer_info.range = bufferSize;

	for (int i = 0; i < MAX_LIGHT_NUM; i++)
	{
		void* light_uniform_buffer_data;
		VkBuffer light_uniform_buffer;
		VkDeviceMemory light_uniform_buffer_memory;
		VkDescriptorBufferInfo light_uniform_buffer_info;

		bufferSize = sizeof(PointLightData);
		CreateUniformBuffer(&light_uniform_buffer_data, (uint32_t)bufferSize, light_uniform_buffer, light_uniform_buffer_memory);
		light_uniform_buffer_info.buffer = light_uniform_buffer;
		light_uniform_buffer_info.offset = 0;
		light_uniform_buffer_info.range = bufferSize;

		light_uniform_buffer_datas.push_back(light_uniform_buffer_data);
		light_uniform_buffers.push_back(light_uniform_buffer);
		light_uniform_buffer_memorys.push_back(light_uniform_buffer_memory);
		light_uniform_buffer_infos.push_back(light_uniform_buffer_info);
	}
}

void VulkanRenderer::CreateDescriptorSetsPool()
{
	std::array<VkDescriptorPoolSize, 5> typeCounts = {};
	typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[0].descriptorCount = swap_chain_images.size() * MAX_MATERIAL_NUM;
	typeCounts[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[1].descriptorCount = swap_chain_images.size() * MAX_MATERIAL_NUM;
	typeCounts[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[2].descriptorCount = swap_chain_images.size() * MAX_MATERIAL_NUM * MAX_LIGHT_NUM;
	typeCounts[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	typeCounts[3].descriptorCount = swap_chain_images.size() * MAX_MATERIAL_NUM;
	typeCounts[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	typeCounts[4].descriptorCount = swap_chain_images.size() * MAX_MATERIAL_NUM;

	VkDescriptorPoolCreateInfo descriptorPool = {};
	descriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPool.pNext = NULL;
	descriptorPool.maxSets = swap_chain_images.size() * MAX_MATERIAL_NUM;
	descriptorPool.poolSizeCount = static_cast<uint32_t>(typeCounts.size());
	descriptorPool.pPoolSizes = typeCounts.data();
	descriptorPool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	vkCreateDescriptorPool(device, &descriptorPool, NULL, &desc_pool);
}

void VulkanRenderer::AllocateDescriptorSets(VkDescriptorSet* descSets)
{
	VkDescriptorSetLayout desc_layouts[3] = { desc_layout, desc_layout, desc_layout };
	VkDescriptorSetAllocateInfo allocInfo[1];
	allocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo[0].pNext = NULL;
	allocInfo[0].descriptorPool = desc_pool;
	allocInfo[0].descriptorSetCount = swap_chain_images.size();
	allocInfo[0].pSetLayouts = desc_layouts;
	vkAllocateDescriptorSets(device, allocInfo, descSets);
}

void VulkanRenderer::FreeDescriptorSets(VkDescriptorSet* descSets)
{
	vkFreeDescriptorSets(device, desc_pool, swap_chain_images.size(), descSets);
}

void VulkanRenderer::CreateCompDescriptorSetsPool()
{
	std::array<VkDescriptorPoolSize, 6> typeCounts = {};
	typeCounts[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	typeCounts[0].descriptorCount = swap_chain_images.size();
	typeCounts[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[1].descriptorCount = swap_chain_images.size();
	typeCounts[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	typeCounts[2].descriptorCount = swap_chain_images.size();
	typeCounts[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	typeCounts[3].descriptorCount = swap_chain_images.size();
	typeCounts[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	typeCounts[4].descriptorCount = swap_chain_images.size();
	typeCounts[5].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	typeCounts[5].descriptorCount = swap_chain_images.size();

	VkDescriptorPoolCreateInfo descriptorPool = {};
	descriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPool.pNext = NULL;
	descriptorPool.maxSets = swap_chain_images.size();
	descriptorPool.poolSizeCount = static_cast<uint32_t>(typeCounts.size());
	descriptorPool.pPoolSizes = typeCounts.data();
	descriptorPool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	vkCreateDescriptorPool(device, &descriptorPool, NULL, &comp_desc_pool);
}

void VulkanRenderer::FreeCompDescriptorSets(VkDescriptorSet* descSets)
{
	vkFreeDescriptorSets(device, comp_desc_pool, swap_chain_images.size(), descSets);
}

void VulkanRenderer::CreateSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &image_available_semaphore) != VK_SUCCESS ||
		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &render_finished_semaphore) != VK_SUCCESS ||
		vkCreateFence(device, &fenceInfo, nullptr, &in_flight_fence) != VK_SUCCESS) {

		throw std::runtime_error("failed to create semaphores!");
	}
	vkResetFences(device, 1, &in_flight_fence);
}

void VulkanRenderer::CreateTextureSampler(VkSampler* sampler)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void VulkanRenderer::DestroyTextureSampler(VkSampler* sampler)
{
	vkDestroySampler(device, *sampler, nullptr);
}

void VulkanRenderer::AddLight(PointLight* light)
{
	PointLightData lightData;
	lightData.color = light->GetColor();
	lightData.pos = light->GetPosition();
	lightData.radius = light->GetRadius();
	lightData.ambient_intensity = light->GetAmbientIntensity();
	lightData.diffuse_intensity = light->GetDiffuseIntensity();
	lightData.specular_intensity = light->GetSpecularIntensity();
	lightData.attenuation_constant = light->GetAttenuationConstant();
	lightData.attenuation_linear = light->GetAttenuationLinear();
	lightData.attenuation_exp = light->GetAttenuationExp();
	light_infos.push_back(lightData);

	int idx = light_infos.size() - 1;
	memcpy(light_uniform_buffer_datas[idx], &lightData, sizeof(PointLightData));
}

void VulkanRenderer::ClearLight()
{
	light_infos.clear();
}

void VulkanRenderer::RenderBegin()
{
	UpdateComputeDescriptorSet();

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(command_buffers[active_command_buffer_idx], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = render_pass;
	renderPassInfo.framebuffer = swap_chain_framebuffers[active_command_buffer_idx];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swap_chain_extent;

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = clear_color.color;
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(command_buffers[active_command_buffer_idx], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(command_buffers[active_command_buffer_idx], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

	/// set camera
	if (camera != NULL)
	{
		camera->UpdateViewProject();

		/// cluster compute parameter setting
		ScreenToView* stv = (ScreenToView*)screen_to_view_buffer_data;
		stv->inverseProjection = glm::inverse(*camera->GetProjectMatrix());
	}
}

void VulkanRenderer::RenderEnd()
{
	vkCmdEndRenderPass(command_buffers[active_command_buffer_idx]);

	if (vkEndCommandBuffer(command_buffers[active_command_buffer_idx]) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void VulkanRenderer::Flush()
{
	if (last_command_buffer_idx != UINT_MAX)
	{
		vkWaitForFences(device, 1, &in_flight_fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		vkResetFences(device, 1, &in_flight_fence);
	
		VkSemaphore waitSemaphores[] = { render_finished_semaphore };
		
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemaphores;
		VkSwapchainKHR swapChains[] = { swap_chain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &last_command_buffer_idx;
		presentInfo.pResults = nullptr; // Optional
		if (vkQueuePresentKHR(graphics_queue, &presentInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present frame buffer!");
		}
	}

	vkAcquireNextImageKHR(device, swap_chain, std::numeric_limits<uint64_t>::max(), image_available_semaphore, VK_NULL_HANDLE, &active_command_buffer_idx);

	Application::Inst()->SceneRender();

	VkSemaphore signalSemaphores[] = { render_finished_semaphore };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { image_available_semaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffers[active_command_buffer_idx];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphics_queue, 1, &submitInfo, in_flight_fence) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	last_command_buffer_idx = active_command_buffer_idx;
}

void VulkanRenderer::WaitIdle()
{
	vkDeviceWaitIdle(device);
}

void VulkanRenderer::OnSceneExit()
{
	ClearLight();
}
