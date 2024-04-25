#include "renderer.h"
#include "glfw_window.h"
#include "logger.h"
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_win32.h>
#include <vector>

dazai_engine::renderer::renderer(glfw_window* window):
	m_window(window)
{
	init();
}

dazai_engine::renderer::~renderer()
{
	vkDestroySurfaceKHR(m_context.instance, m_context.surface, nullptr);
	vkDestroyInstance(m_context.instance, nullptr);
	vkDestroyDevice(m_context.device, nullptr);
}

auto dazai_engine::renderer::init() -> void
{
	//app info
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Dazai Vulkan";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "Dazai Engine";
	app_info.apiVersion = VK_API_VERSION_1_0;
	//global extensions and validation layer
	const char* layers[]
	{
		"VK_LAYER_KHRONOS_validation"
	};
	VkInstanceCreateInfo instance_info{};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	//glfw extensions
	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
	std::vector<const char*> extensions(glfw_extensions,
		glfw_extensions + glfw_extension_count);
	//add other extensions
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	instance_info.ppEnabledExtensionNames = extensions.data();
	instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instance_info.ppEnabledLayerNames = layers;
	instance_info.enabledLayerCount = ARRAYSIZE(layers);
	//CREATE INSTANCE
	VKCHECK(vkCreateInstance(&instance_info, nullptr, &m_context.instance));
	//ENABLE DEBUG MESSENGER
	//STEPS:
	//GET FUNCTION POINTER FROM DLL
	//CAST THE FUNCTION APPROPRIATELY
	//IF POINTER IS VALID CREATE MESSENGER
	auto debug_messenger_function_ptr =
		(PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(m_context.instance, "vkCreateDebugUtilsMessengerEXT");
	if (debug_messenger_function_ptr)
	{
		VkDebugUtilsMessengerCreateInfoEXT debug_info{};
		debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		//turn on bits for message type
		debug_info.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debug_info.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		//callback functiion
		debug_info.pfnUserCallback = VK_DEBUG_CALLBACK;
		debug_messenger_function_ptr(m_context.instance, &debug_info,
			0, &m_context.debug_messenger);
	}
	else
	{
		LOG_ERROR("DEBUG MESSENGER FUNCTION PTR NOT FOUND");
	}
	//create vulkan surface
	VkWin32SurfaceCreateInfoKHR surface_info{};
	surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hwnd = glfwGetWin32Window(m_window->window);
	surface_info.hinstance = GetModuleHandle(nullptr);
	VKCHECK(vkCreateWin32SurfaceKHR(m_context.instance, &surface_info, 0, &m_context.surface));
	//select physical device
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(m_context.instance, &device_count, nullptr);
	if (device_count == 0)
		LOG_ERROR("No GPU with Vulkan support found");
	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(m_context.instance, &device_count, devices.data());
	if (device_count > 1)
		LOG_WARNING("You have more than one supported physical devices, set accordingly");
	//I only have 1 device so setting directly
	m_context.physical_device = devices[0];
	//check queue families in selected gpu
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_context.physical_device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_context.physical_device, &queue_family_count, queue_families.data());
	int i = 0;
	for (const auto& queue_family: queue_families)
	{
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) // check if queue family has graphic queue
			m_context.graphic_family_queue_index = i;

		if (m_context.graphic_family_queue_index.has_value()) // if value was assigned break
			break;

		i++;
	}
	if (!m_context.graphic_family_queue_index.has_value())
	{
		LOG_ERROR("Graphic queue family failed");
		return;
	}
	//CREATE LOGICAL DEVICE
	//first we need to configure queue family we going to use in logical device
	VkDeviceQueueCreateInfo queue_create_info{};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = m_context.graphic_family_queue_index.value();
	queue_create_info.queueCount = 1;
	float queue_priority = 1;
	queue_create_info.pQueuePriorities = &queue_priority;
	//configure physical device feature we will be using;
	VkPhysicalDeviceFeatures device_features{}; // nothing for now
	//create extensions for logical device
	const char* sc_extensions[] = 
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	//now create logical device
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pEnabledFeatures = &device_features;
	device_create_info.ppEnabledExtensionNames = sc_extensions;
	device_create_info.enabledExtensionCount = ARRAYSIZE(sc_extensions);
	VKCHECK(vkCreateDevice(m_context.physical_device,
		&device_create_info,0,&m_context.device));
	//Retrieving queue handles
	vkGetDeviceQueue(m_context.device,m_context.graphic_family_queue_index.value(),
		0,&m_context.graphics_queue);
	//CREATE SWAP CHAIN
	//get surface capabilities for pre swapchain config data
	VkSurfaceCapabilitiesKHR surface_capabilities{};
	VKCHECK( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context.physical_device,
		m_context.surface, &surface_capabilities));
	uint32_t surface_img_count = surface_capabilities.minImageCount + 1;
	surface_img_count =
		surface_img_count > surface_capabilities.maxImageCount ?
		surface_img_count - 1 : surface_img_count;
	//get surface format
	uint32_t format_count = 0;
	VKCHECK( vkGetPhysicalDeviceSurfaceFormatsKHR(m_context.physical_device, m_context.surface,
		&format_count, 0));
	std::vector<VkSurfaceFormatKHR> formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_context.physical_device, m_context.surface,
		&format_count, formats.data());
	for (auto format: formats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB)
		{
			m_context.surface_format = format;
			break;
		}		
	}
	VkSwapchainCreateInfoKHR sc_info{};
	sc_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sc_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sc_info.surface = m_context.surface;
	sc_info.preTransform = surface_capabilities.currentTransform;
	sc_info.imageExtent = surface_capabilities.currentExtent;
	sc_info.minImageCount = surface_img_count;
	sc_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	sc_info.imageArrayLayers = 1;
	sc_info.imageFormat = m_context.surface_format.format;
	VKCHECK(vkCreateSwapchainKHR(m_context.device, &sc_info, 0,
		&m_context.swap_chain));
	//GET SWAP CHAIN IMAGES
	VKCHECK( vkGetSwapchainImagesKHR(m_context.device, m_context.swap_chain, 
		&m_context.sc_image_count, 0));
	//resize the images vector
	m_context.sc_images.resize(m_context.sc_image_count);
	//now assign swap chain images
	VKCHECK(vkGetSwapchainImagesKHR(m_context.device, m_context.swap_chain,
		&m_context.sc_image_count, m_context.sc_images.data()));
	//SWAP CHAIN IMAGE VIEWS
	VkImageViewCreateInfo iv_info{};
	iv_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	iv_info.format = m_context.surface_format.format;
	iv_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	iv_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	iv_info.subresourceRange.layerCount = 1;
	iv_info.subresourceRange.levelCount = 1;
	m_context.sc_image_views.resize(m_context.sc_image_count);
	for (size_t i = 0; i < m_context.sc_image_count; i++)
	{
		iv_info.image = m_context.sc_images[i];
		VKCHECK (vkCreateImageView(m_context.device, &iv_info,
			0, &m_context.sc_image_views[i]));
	}

	//RENDER PASS
	VkRenderPassCreateInfo rp_info{};
	VkAttachmentDescription attachment{};
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.format = m_context.surface_format.format;
	//subpass description
	VkAttachmentReference color_attachment{};
	color_attachment.attachment = 0;
	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass_desc{};
	subpass_desc.colorAttachmentCount = 1;
	subpass_desc.pColorAttachments = &color_attachment;
	VkAttachmentDescription attachments[]
	{
		attachment
	};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.attachmentCount = 1;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = ARRAYSIZE(attachments);
	rp_info.pSubpasses = &subpass_desc;
	VKCHECK (vkCreateRenderPass(m_context.device, &rp_info ,
		0, &m_context.render_pass));
	//FRAMEBUFFER
	VkFramebufferCreateInfo fb_info{};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.renderPass = m_context.render_pass;
	fb_info.layers = 1;
	fb_info.attachmentCount = 1;
	fb_info.width = m_window->width;
	fb_info.height = m_window->height;
	m_context.frame_buffers.resize(m_context.sc_image_count);
	for (size_t i = 0; i < m_context.sc_image_count; i++)
	{
		fb_info.pAttachments = &m_context.sc_image_views[i];
		VKCHECK( vkCreateFramebuffer(m_context.device, &fb_info,
			0, &m_context.frame_buffers[i]));
	}
	//COMMAND POOL
	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = m_context.graphic_family_queue_index.value();
	vkCreateCommandPool(m_context.device,&pool_info,0,
		&m_context.command_pool);
	//SEMAPHORES
	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VKCHECK(vkCreateSemaphore(m_context.device,&semaphore_info,0,
		&m_context.acquire_semaphore));
	VKCHECK( vkCreateSemaphore(m_context.device,&semaphore_info,0,
		&m_context.submit_semaphore));
	//FENCES
	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VKCHECK(vkCreateFence(m_context.device,&fence_info,0,&m_context.submit_queue_fence));
}

auto dazai_engine::renderer::render() -> bool
{
	//ACQUIRE SWAPCHAIN IMAGE
	uint32_t image_idx;
	VKCHECK( vkAcquireNextImageKHR(m_context.device,m_context.swap_chain
		,0,m_context.acquire_semaphore,0,&image_idx));
	//allocate command buffer
	VkCommandBuffer cmd;
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = m_context.command_pool;
	vkAllocateCommandBuffers(m_context.device,&alloc_info, &cmd);
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VKCHECK( vkBeginCommandBuffer(cmd, &begin_info));
	VkClearValue clear_value{};
	clear_value.color = { 1,1,0,1 };
	//renderpass begin
	VkRenderPassBeginInfo rp_begin_info{};
	rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_begin_info.renderPass = m_context.render_pass;
	VkExtent2D screen_size = {m_window->width,m_window->height};
	rp_begin_info.renderArea.extent = screen_size;
	rp_begin_info.framebuffer = m_context.frame_buffers[image_idx];
	rp_begin_info.pClearValues = &clear_value;
	rp_begin_info.clearValueCount = 1;
	vkCmdBeginRenderPass(cmd, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	//RENDERING COMMANDS
	{
		
	}
	vkCmdEndRenderPass(cmd);
	VKCHECK(vkEndCommandBuffer(cmd));
	//RESET SUBMIT FENCE FIRST
	VKCHECK(vkResetFences(m_context.device,1, &m_context.submit_queue_fence));
	//SUMBIT
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &m_context.acquire_semaphore;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &m_context.submit_semaphore;
	//assign wait stage mask for submit request
	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit_info.pWaitDstStageMask = &wait_stage;
	vkQueueSubmit(m_context.graphics_queue,1,&submit_info, m_context.submit_queue_fence);
	//wait for fence
	VKCHECK(vkWaitForFences(m_context.device,1,&m_context.submit_queue_fence, VK_TRUE, UINT64_MAX));
	//PRESENT
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pSwapchains = &m_context.swap_chain;
	present_info.swapchainCount = 1;
	present_info.pImageIndices = &image_idx;
	present_info.pWaitSemaphores = &m_context.submit_semaphore;
	present_info.waitSemaphoreCount = 1;
	VKCHECK(vkQueuePresentKHR(m_context.graphics_queue, &present_info));

	//FREE COMMAND BUFFER
	vkFreeCommandBuffers(m_context.device,
		m_context.command_pool, 1, &cmd);

	return true;
}



