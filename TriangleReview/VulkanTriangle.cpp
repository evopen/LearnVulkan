#include "VulkanTriangle.h"
#include <iostream>
#include <fstream>

VkVertexInputBindingDescription Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);
	return attributeDescriptions;
}

void VulkanTriangle::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void VulkanTriangle::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "numerous", nullptr, nullptr);
}

void VulkanTriangle::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapchain();
	createImageView();
	createShaderModule();
	createRenderPass();
	createPipeline();
	createFramebuffers();
	createCommandPool();
	createVertexBuffer();
	createCommandBuffers();
	createSyncObjects();
}

void VulkanTriangle::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		drawFrame();
	}
}

void VulkanTriangle::cleanup()
{
	glfwDestroyWindow(window);
}

std::vector<const char*> VulkanTriangle::getRequiredExtensions()
{
	uint32_t extensionCount = 0;
	const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	std::vector<const char*> requiredExtensions(glfwRequiredExtensions, glfwRequiredExtensions + extensionCount);
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	return requiredExtensions;
}

VkBool32 VulkanTriangle::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageType,
                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                       void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl << std::endl;

	return VK_FALSE;
}

void VulkanTriangle::createInstance()
{
	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	applicationInfo.pApplicationName = "Hello Triangle";
	applicationInfo.pEngineName = "no engine";

	const std::vector<const char*> requiredExtensions = getRequiredExtensions();
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
	instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
	debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugUtilsMessengerCreateInfo.pfnUserCallback = &debugCallback;
	instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;

	vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
}

void VulkanTriangle::setupDebugMessenger()
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {};
	debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	debugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugUtilsMessengerCreateInfo.pfnUserCallback = &debugCallback;

	if (func(instance, &debugUtilsMessengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to setup debug messenger");
	}
}

void VulkanTriangle::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	for (const auto& device : devices)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			physicalDevice = device;
			std::cout << "Pick GPU: " << properties.deviceName << std::endl;
			break;
		}
	}
}

void VulkanTriangle::createLogicalDevice()
{
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
	for (int i = 0; i < queueFamilyProperties.size(); i++)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsQueueIndex = i;
		}
	}
	if (!graphicsQueueIndex.has_value())
	{
		throw std::runtime_error("Failed to find suitable queue family");
	}
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.queueFamilyIndex = graphicsQueueIndex.value();
	float queuePriority = 1.f;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device");
	}
	vkGetDeviceQueue(device, graphicsQueueIndex.value(), 0, &graphicsQueue);
}

void VulkanTriangle::createSwapchain()
{
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create surface");
	};
	VkBool32 presentSupport;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsQueueIndex.value(), surface, &presentSupport);
	if (!presentSupport)
	{
		std::runtime_error("graphics queue does not support presentation");
	}
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
	swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
	swapchainCreateInfo.presentMode = presetnMode;
	swapchainCreateInfo.imageFormat = imageFormat;
	if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swapchain");
	}


	vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
	swapchainImages.resize(swapchainImageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());
	extent = surfaceCapabilities.currentExtent;
}

void VulkanTriangle::createImageView()
{
	swapchainImageViews.resize(swapchainImageCount);

	for (size_t i = 0; i < swapchainImageCount; ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = imageFormat;

		vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]);
	}
}

void VulkanTriangle::createShaderModule()
{
	VkShaderModuleCreateInfo vertShaderModuleCreateInfo = {};
	vertShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	std::vector<char> vertShaderCode = readFile("D:/Dev/LearnVulkan/Triangle/shaders/vert.spv");
	vertShaderModuleCreateInfo.codeSize = vertShaderCode.size();
	vertShaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(vertShaderCode.data());
	vkCreateShaderModule(device, &vertShaderModuleCreateInfo, nullptr, &vertShaderModule);

	VkShaderModuleCreateInfo fragShaderModuleCreateInfo = {};
	fragShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	std::vector<char> fragShaderCode = readFile("D:/Dev/LearnVulkan/Triangle/shaders/frag.spv");
	fragShaderModuleCreateInfo.codeSize = fragShaderCode.size();
	fragShaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(fragShaderCode.data());
	vkCreateShaderModule(device, &fragShaderModuleCreateInfo, nullptr, &fragShaderModule);
}

void VulkanTriangle::createPipeline()
{
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	VkPipelineShaderStageCreateInfo vertStageCreateInfo = {};
	vertStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertStageCreateInfo.module = vertShaderModule;
	vertStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragStageCreateInfo = {};
	fragStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragStageCreateInfo.module = fragShaderModule;
	fragStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo stages[] = {vertStageCreateInfo, fragStageCreateInfo};

	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = stages;

	VkPipelineRasterizationStateCreateInfo rasterizaterCreateInfo = {};
	rasterizaterCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizaterCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizaterCreateInfo.lineWidth = 1.f;

	pipelineCreateInfo.pRasterizationState = &rasterizaterCreateInfo;
	pipelineCreateInfo.renderPass = renderPass;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

	pipelineCreateInfo.layout = pipelineLayout;

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

	VkViewport viewport = {};
	viewport.width = WIDTH;
	viewport.height = HEIGHT;
	// viewport.maxDepth = 1.f;
	VkRect2D scissor = {};
	scissor.extent = extent;

	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.pViewports = &viewport;
	pipelineViewportStateCreateInfo.scissorCount = 1;
	pipelineViewportStateCreateInfo.pScissors = &scissor;

	pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;

	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
	multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;

	VkPipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo = {};
	assemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineCreateInfo.pInputAssemblyState = &assemblyStateCreateInfo;

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();
	vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlenCreateInfo = {};
	colorBlenCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlenCreateInfo.attachmentCount = 1;
	colorBlenCreateInfo.pAttachments = &colorBlendAttachmentState;
	pipelineCreateInfo.pColorBlendState = &colorBlenCreateInfo;

	vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
}

std::vector<char> VulkanTriangle::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

void VulkanTriangle::createRenderPass()
{
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	VkAttachmentDescription attachmentDescription = {};
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.format = imageFormat;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	VkAttachmentReference attachmentRef = {};
	attachmentRef.attachment = 0;
	attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &attachmentRef;

	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;

	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;

	if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass");
	}
}

void VulkanTriangle::createFramebuffers()
{
	framebuffers.resize(swapchainImageCount);
	for (size_t i = 0; i < swapchainImageCount; i++)
	{
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.width = WIDTH;
		framebufferCreateInfo.height = HEIGHT;
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &swapchainImageViews[i];

		vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]);
	}
}

void VulkanTriangle::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = graphicsQueueIndex.value();
	vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);
}

void VulkanTriangle::createCommandBuffers()
{
	commandBuffers.resize(swapchainImages.size());
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = swapchainImageCount;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = commandPool;
	vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data());

	for (size_t i = 0; i < swapchainImageCount; i++)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);


		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffers[i];
		renderPassBeginInfo.renderArea.extent = extent;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, &offset);
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);
		vkEndCommandBuffer(commandBuffers[i]);
	}
}

void VulkanTriangle::createSyncObjects()
{
	imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	submitFences.resize(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore[i]);
		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore[i]);

		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(device, &fenceCreateInfo, nullptr, &submitFences[i]);
	}
}

void VulkanTriangle::createVertexBuffer()
{
	VkDeviceSize size = sizeof(vertices[0]) * vertices.size();
	createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferMemroy);

	void* data;
	vkMapMemory(device, vertexBufferMemroy, 0, size, 0, &data);
	memcpy(data, vertices.data(), (size_t)size);
	vkUnmapMemory(device, vertexBufferMemroy);
}

void VulkanTriangle::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer,
                                  VkMemoryPropertyFlags memoryPropertyFlags,
                                  VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);
	vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory);
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanTriangle::drawFrame()
{
	vkWaitForFences(device, 1, &submitFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &submitFences[currentFrame]);
	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE,
	                      &imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	submitInfo.waitSemaphoreCount = 1;
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphore[currentFrame]};
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphore[currentFrame]};
	submitInfo.pWaitSemaphores = waitSemaphores;
	VkPipelineStageFlags waitPipelineStages = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
	submitInfo.pWaitDstStageMask = &waitPipelineStages;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, submitFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit command");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pWaitSemaphores = signalSemaphores;
	vkQueuePresentKHR(graphicsQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

uint32_t VulkanTriangle::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type");
}
