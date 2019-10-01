#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>
#include <string>
#include <vector>
#include <optional>

struct QueueFamilyIndex
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
	}
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanBase
{
public:
	const std::vector<const char*> extraLayers = {
		"VK_LAYER_LUNARG_monitor"
	};
	uint32_t windowWidth = 800;
	uint32_t windowHeight = 600;
	GLFWwindow* window;
	std::string appName = "numerous";
	std::string windowTitle = appName;
	const uint32_t appVersion = VK_MAKE_VERSION(0, 0, 1);
	const uint32_t engineVersion = VK_MAKE_VERSION(0, 0, 1);
	bool enableValidation = true;
	QueueFamilyIndex queueFamilyIndex;

private:
	VkDebugUtilsMessengerEXT debugMessenger;

public:
	VmaAllocator allocator;
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	VkRenderPass renderPass;
	VkSurfaceFormatKHR surfaceFormat;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkImage colorImage;
	VmaAllocation colorImageAllocation;
	VkImageView colorImageView;
	VkImage depthImage;
	VmaAllocation depthImageAllocation;
	VkImageView depthImageView;
	VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_8_BIT;
	VkFormat depthImageFormat = VK_FORMAT_D32_SFLOAT;
	std::vector<VkFramebuffer> swapchainFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkCommandBuffer> commandBuffers;
	VkQueue graphicsQueue;
	VkQueue transferQueue;
	VkQueue presentQueue;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VmaAllocation> uniformBufferAllocation;
	size_t currentFrame = 0;

public:


	explicit VulkanBase(bool enableValidation)
		: enableValidation(enableValidation)
	{
	}

	void init();

private:
	void initWindow();
	void initVulkan();

private:
	void createInstance();
	void setupDebugMessenger();
	void pickPhysicalDevice();
	void findQueueFamilyIndex();
	void createLogicalDevice();
	void createMemoryAllocator();
	void createSwapchain();
	void createSwapchainImageViews();
	void createRenderPass();
	void createColorResources();
	void createDepthResources();
	void createFramebuffers();
	void createCommandPool();
	void createSyncObjects();
	void createDescriptorSetLayout();
	void createDescriptorPool();

public:
	void drawFrame();
	void createUniformBuffer(VkDeviceSize bufferSize);

	virtual void createGraphicsPipeline() = 0;
	virtual void createCommandBuffers() = 0;
	virtual void updateUniformBuffer(uint32_t currentImage) = 0;

private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	std::vector<const char*> getRequiredExtensions();
	std::vector<const char*> getRequiredLayers();
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlagBits aspectFlags, uint32_t mipLevels);
	VkSurfaceFormatKHR chooseSurfaceFormat();
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
	                  VkBuffer& buffer, VmaAllocation& allocation);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevelCount, VkSampleCountFlagBits sampleCount,
	                 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage,
	                 VkImage& image, VmaAllocation& allocation);
	static std::vector<char> readFile(const std::string& filename);

public:
	VkShaderModule createShaderModule(const std::string& filename);
};
