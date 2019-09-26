#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <string>
#include <glm/glm.hpp>
#include <array>

const int WIDTH = 800;
const int HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription();
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};

const std::vector<Vertex> vertices = {
	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_LUNARG_monitor"
};
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
const VkPresentModeKHR presetnMode = VK_PRESENT_MODE_MAILBOX_KHR;

class VulkanTriangle
{
private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	std::optional<uint32_t> graphicsQueueIndex;
	VkSwapchainKHR swapchain;
	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	uint32_t swapchainImageCount;
	VkRenderPass renderPass;
	std::vector<VkSemaphore> imageAvailableSemaphore;
	std::vector<VkSemaphore> renderFinishedSemaphore;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkFramebuffer> framebuffers;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	uint32_t currentFrame = 0;
	std::vector<VkFence> submitFences;
	VkPipeline pipeline;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	VkPipelineLayout pipelineLayout;
	VkExtent2D extent;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemroy;


public:
	void run();

private:
	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();

private:
	std::vector<const char*> getRequiredExtensions();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	void createInstance();
	void setupDebugMessenger();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapchain();
	void createImageView();
	void createShaderModule();
	void createPipeline();
	void createRenderPass();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncObjects();
	void createVertexBuffer();
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer,
	                  VkMemoryPropertyFlags memoryPropertyFlags,
	                  VkDeviceMemory& bufferMemory);
	void drawFrame();

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	std::vector<char> readFile(const std::string& filename);
};
