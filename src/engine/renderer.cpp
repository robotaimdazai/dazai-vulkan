#include "renderer.h"
#include "glfw_window.h"
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_win32.h>
#include <vector>
#include "resources.h"
#include "logger.h"

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

auto dazai_engine::renderer::init() -> bool
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
		return false;
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
	//####################################################
	//################### PIEPLEINES ######################
	// ###################################################
	//vertex input
	VkPipelineVertexInputStateCreateInfo vi_info{};
	vi_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vi_info.vertexAttributeDescriptionCount = 0;
	vi_info.vertexBindingDescriptionCount = 0;
	//input assembly
	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	//color attachments
	VkPipelineColorBlendAttachmentState c_attachments{};
	c_attachments.blendEnable = VK_FALSE; // to enable alpha blending
	c_attachments.colorWriteMask = 
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	//color blend state
	VkPipelineColorBlendStateCreateInfo cb_info{};
	cb_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb_info.pAttachments = &c_attachments;
	cb_info.attachmentCount = 1;
	//SHADER STAGE
	VkShaderModule v_module, f_module;
	//vertex shader info
	uint32_t v_size_bytes;
	//TODO: abstract shaders and refactor
	char* v_code = resources::
		read_raw_file("shaders/default.vert.spv", &v_size_bytes);
	VkShaderModuleCreateInfo vs_info{};
	vs_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vs_info.pCode = (uint32_t*)v_code;
	vs_info.codeSize = v_size_bytes;
	VKCHECK( vkCreateShaderModule(m_context.device,&vs_info,0,&v_module));
	delete v_code;
	//fragment shader info
	uint32_t f_size_bytes;
	char* f_code = resources::
		read_raw_file("shaders/default.frag.spv", &f_size_bytes);
	VkShaderModuleCreateInfo fs_info{};
	fs_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fs_info.pCode = (uint32_t*)f_code;
	fs_info.codeSize = f_size_bytes;
	VKCHECK( vkCreateShaderModule(m_context.device,&fs_info,0,&f_module));
	delete f_code;
	//vertex stage
	VkPipelineShaderStageCreateInfo v_stage{};
	v_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	v_stage.pName = "main"; // main fn in shader
	v_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	v_stage.module = v_module;
	//fragment stage
	VkPipelineShaderStageCreateInfo f_stage{};
	f_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	f_stage.pName = "main"; // main fn in shader
	f_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	f_stage.module = f_module;
	VkPipelineShaderStageCreateInfo shader_stages[]{
		v_stage,
		f_stage
	};
	
	//viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_window->width;
	viewport.height = (float)m_window->height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	//scissor
	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = { m_window->width,m_window->height };
	//dynamic state for viewports 
	VkDynamicState dynamic_states[]{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamic_state{};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = ARRAYSIZE(dynamic_states);
	dynamic_state.pDynamicStates = dynamic_states;
	//viewport state
	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.pScissors = &scissor;

	//rasterization stage
	VkPipelineRasterizationStateCreateInfo rasterization_state{};
	rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization_state.lineWidth = 1.0f;
	rasterization_state.depthClampEnable = VK_FALSE;
	rasterization_state.depthBiasClamp = VK_FALSE;
	rasterization_state.rasterizerDiscardEnable = VK_FALSE;
	//multi sampling
	VkPipelineMultisampleStateCreateInfo msa_info{};
	msa_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msa_info.sampleShadingEnable = VK_FALSE;
	msa_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	//layouts for uniforms
	VkPipelineLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	VKCHECK(vkCreatePipelineLayout(m_context.device,&layout_info,
		0,&m_context.pipeline_layout));
	//main pipeline config
	VkGraphicsPipelineCreateInfo p_info{};
	p_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	p_info.pColorBlendState = &cb_info;
	p_info.pVertexInputState = &vi_info;
	p_info.pStages = shader_stages;
	p_info.stageCount = ARRAYSIZE(shader_stages);
	p_info.renderPass = m_context.render_pass;
	p_info.pViewportState = &viewport_state;
	p_info.pInputAssemblyState = &input_assembly;
	p_info.pRasterizationState = &rasterization_state;
	p_info.pMultisampleState = &msa_info;
	p_info.layout = m_context.pipeline_layout;
	vkCreateGraphicsPipelines(m_context.device,0,
		1,&p_info,0,&m_context.pipeline );

	//########################################################
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
	VKCHECK(vkCreateFence(m_context.device,&fence_info,0,
		&m_context.submit_queue_fence));

	// load image
	{
		DDSFile* data = resources::load_dds_file("textures/ball.dds");
		//TODO:Abstract the image loading
		VkImageCreateInfo image_info{};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
		image_info.extent = { data->header.Width,data->header.Height,1 };
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.usage =
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | // image will be used for transferring data from cpu-gpu
			VK_IMAGE_USAGE_SAMPLED_BIT; // image will be used for sampling in frag shader
		//image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VKCHECK(vkCreateImage(m_context.device, &image_info, 0,
			&m_context.image.vk_image));

		VkMemoryRequirements mem_req{};
		vkGetImageMemoryRequirements(m_context.device, m_context.image.vk_image, &mem_req);
		VkPhysicalDeviceMemoryProperties mem_prop{};
		vkGetPhysicalDeviceMemoryProperties(m_context.physical_device, &mem_prop);
		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = data->header.Width * data->header.Height * 4;//4 = rgba
		for (size_t i = 0; i < mem_prop.memoryTypeCount; i++)
		{
			if (mem_req.memoryTypeBits & (1 << i) &&
				(mem_prop.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
				== VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				alloc_info.memoryTypeIndex = i;
			}
		}

		VKCHECK(vkAllocateMemory(m_context.device, &alloc_info, 0,
			&m_context.image.memory));
		VKCHECK(vkBindImageMemory(m_context.device,
			m_context.image.vk_image, m_context.image.memory, 0));

	}
	return true;
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
		VkRect2D scissor{};
		scissor.extent = { m_window->width,m_window->height };
		VkViewport viewport{};
		viewport.width = m_window->width;
		viewport.height = m_window->height;
		viewport.maxDepth = 1.0f;
		vkCmdSetScissor(cmd,0,1,&scissor);
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdBindPipeline(cmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS, m_context.pipeline);
		vkCmdDraw(cmd, 3, 1, 0, 0);
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



