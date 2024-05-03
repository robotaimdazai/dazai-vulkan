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
	//crete vulkan surface
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

	//DESCRIPTOR SET
	//binding
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = 0;
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = 1;
		layout_info.pBindings = &binding;
		vkCreateDescriptorSetLayout(m_context.device, &layout_info, 0,
			&m_context.set_layout);
	}
	//layouts for uniforms
	VkPipelineLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_info.setLayoutCount = 1;
	layout_info.pSetLayouts = &m_context.set_layout;
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
	vkDestroyShaderModule(m_context.device, v_module, 0);
	vkDestroyShaderModule(m_context.device, f_module, 0);

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
	VkFenceCreateInfo f_info = fence_info(VK_FENCE_CREATE_SIGNALED_BIT);
	VKCHECK(vkCreateFence(m_context.device,&f_info,0,
		&m_context.submit_queue_fence));

	//STAGING BUFFER
	m_context.staging_buffer = alloc_buffer(
		m_context.device,
		m_context.physical_device,
		MB(10),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);

	// load image
	{
		DDSFile* data = resources::load_dds_file("textures/ball.dds");
		uint32_t texture_size = data->header.Width * data->header.Height * 4;//4 = rgba
		copy_to_buffer(&m_context.staging_buffer,&data->dataBegin,texture_size);
		
		//TODO:Abstract` the image loading
		m_context.image = alloc_image(m_context.device,m_context.physical_device,
			data->header.Width,data->header.Height,VK_FORMAT_R8G8B8A8_UNORM);
		VkCommandBuffer cmd;
		VkCommandBufferAllocateInfo cmd_alloc = cmd_alloc_info(m_context.command_pool);
		VKCHECK( vkAllocateCommandBuffers(m_context.device,
			&cmd_alloc,&cmd));
		VkCommandBufferBeginInfo begin_info = cmd_begin_info();
		vkBeginCommandBuffer(cmd, &begin_info);

		VkImageSubresourceRange range{};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.levelCount = 1;
		range.layerCount = 1;
		//transition layout to transfer optimal
		VkImageMemoryBarrier image_barrier{};
		image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_barrier.image = m_context.image.vk_image;

		image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_barrier.srcAccessMask = 0;
		image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_barrier.subresourceRange = range;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, 0, 0,
			0, 1, &image_barrier);
		
		VkBufferImageCopy copy_region{};
		copy_region.imageExtent = { data->header.Width, data->header.Height,1 };
		copy_region.imageSubresource.layerCount = 1;
		copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vkCmdCopyBufferToImage(cmd,m_context.staging_buffer.vk_buffer,
			m_context.image.vk_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&copy_region);

		image_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, 0, 0,
			0, 1, &image_barrier);

		vkEndCommandBuffer(cmd);

		VkFence upload_fence; 
		VkFenceCreateInfo upload_fence_info = fence_info();
		VKCHECK(vkCreateFence(m_context.device, &upload_fence_info,
			0,&upload_fence));

		VkSubmitInfo sub_info = submit_info(&cmd);
		vkQueueSubmit(m_context.graphics_queue, 1, &sub_info, upload_fence);
		VKCHECK( vkWaitForFences(m_context.device,1,&upload_fence,
			true,UINT64_MAX));
	}
	//image view
	{
		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = m_context.image.vk_image;
		view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.layerCount = 1;
		view_info.subresourceRange.levelCount = 1;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

		VKCHECK( vkCreateImageView(m_context.device, &view_info,
			0, &m_context.image.view));
	}
	//create sampler
	{
		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.minFilter = VK_FILTER_NEAREST;
		sampler_info.magFilter = VK_FILTER_NEAREST;

		VKCHECK( vkCreateSampler(m_context.device, &sampler_info, 
			0, &m_context.sampler));
	}
	//descriptor pool
	{
		VkDescriptorPoolSize pool_size{};
		pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_size.descriptorCount = 1;

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.maxSets = 1;
		pool_info.poolSizeCount = 1;
		pool_info.pPoolSizes = &pool_size;
		vkCreateDescriptorPool(m_context.device, &pool_info,
			0, &m_context.descriptor_pool);
	}
	//create descriptor set
	{
		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.pSetLayouts = &m_context.set_layout;
		alloc_info.descriptorSetCount = 1;
		alloc_info.descriptorPool = m_context.descriptor_pool;
		vkAllocateDescriptorSets(m_context.device,&alloc_info,&m_context.descriptor_set);
	}
	//update descriptor set
	VkDescriptorImageInfo image_info{};
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image_info.imageView = m_context.image.view;
	image_info.sampler = m_context.sampler;
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = m_context.descriptor_set;
	write.pImageInfo = &image_info;
	write.dstBinding = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.descriptorCount = 1;
	vkUpdateDescriptorSets(m_context.device,1,&write,0,0);
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
	VkCommandBufferAllocateInfo alloc_info = cmd_alloc_info(m_context.command_pool);
	VKCHECK(vkAllocateCommandBuffers(m_context.device,&alloc_info, &cmd));
	VkCommandBufferBeginInfo begin_info = cmd_begin_info();
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
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd,0,1,&scissor);

		vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_context.pipeline_layout,
			0,1, &m_context.descriptor_set 
			,0,0);

		vkCmdBindPipeline(cmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS, m_context.pipeline);
		vkCmdDraw(cmd, 6, 1, 0, 0);
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

//###################	PRIVATE HELPERS	#########################

auto dazai_engine::renderer::cmd_begin_info() -> VkCommandBufferBeginInfo
{
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	return info;
}

auto dazai_engine::renderer::cmd_alloc_info(VkCommandPool pool) -> VkCommandBufferAllocateInfo
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandBufferCount = 1;
	info.commandPool = pool;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	return info;
}

auto dazai_engine::renderer::fence_info(VkFenceCreateFlags flags) -> VkFenceCreateInfo
{
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.flags = flags;
	return info;
}


auto dazai_engine::renderer::submit_info(VkCommandBuffer* cmd, uint32_t cmd_count) -> VkSubmitInfo
{
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.commandBufferCount = cmd_count;
	info.pCommandBuffers = cmd;

	return info;
}

auto dazai_engine::renderer::alloc_image
(VkDevice device,
	VkPhysicalDevice physical_device,
	uint32_t width,
	uint32_t height,
	VkFormat format) -> image
{
	image image{};
	VkImageCreateInfo image_info{};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = format;
	image_info.extent = { width,height,1 };
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.usage =
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | // image will be used for transferring data from cpu-gpu
		VK_IMAGE_USAGE_SAMPLED_BIT; // image will be used for sampling in frag shader
	//image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VKCHECK(vkCreateImage(device, &image_info, 0,
		&image.vk_image));

	VkMemoryRequirements mem_req{};
	vkGetImageMemoryRequirements(device, image.vk_image, &mem_req);
	VkPhysicalDeviceMemoryProperties mem_prop{};
	vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_prop);
	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_req.size;
	alloc_info.memoryTypeIndex = get_memory_type_index(physical_device,mem_req,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VKCHECK(vkAllocateMemory(device, &alloc_info, 0,
		&image.memory));
	VKCHECK(vkBindImageMemory(device,
		image.vk_image, image.memory, 0));

	return image;
}

auto dazai_engine::renderer::alloc_buffer(
	VkDevice device,
	VkPhysicalDevice physical_device,
	uint32_t size,
	VkBufferUsageFlags buffer_usage,
	VkMemoryPropertyFlags mem_props) -> buffer
{
	buffer buffer{};
	buffer.size = size;
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = buffer_usage;
	buffer_info.size = size;
	VKCHECK(vkCreateBuffer(device, &buffer_info, 0,
		&buffer.vk_buffer));

	
	VkMemoryRequirements mem_req{};
	vkGetBufferMemoryRequirements(device, buffer.vk_buffer,
		&mem_req);
	VkPhysicalDeviceMemoryProperties mem_prop{};
	vkGetPhysicalDeviceMemoryProperties(physical_device,
		&mem_prop);
	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = buffer.size;
	alloc_info.memoryTypeIndex = 
		get_memory_type_index(physical_device,
		mem_req,
		mem_props);

	VKCHECK(vkAllocateMemory(device, &alloc_info, 0,
		&buffer.memory));
	//only map when we can write to memory from cpu
	if (mem_props  & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)

	{
		VKCHECK(vkMapMemory(device, buffer.memory, 0, MB(10),
			0, &buffer.data));
	}
	VKCHECK(vkBindBufferMemory(device, buffer.vk_buffer,
		buffer.memory, 0));
	
	return buffer;
}


auto dazai_engine::renderer::get_memory_type_index(
	VkPhysicalDevice physical_device,
	VkMemoryRequirements mem_reqs,
	VkMemoryPropertyFlags mem_props) -> uint32_t
{
	uint32_t type_index = UINT32_MAX;
	VkPhysicalDeviceMemoryProperties mem_prop{};
	vkGetPhysicalDeviceMemoryProperties(physical_device,
		&mem_prop);
	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_reqs.size;
	for (size_t i = 0; i < mem_prop.memoryTypeCount; i++)
	{
		if (mem_reqs.memoryTypeBits & (1 << i) &&
			(mem_prop.memoryTypes[i].propertyFlags &
				mem_props)
			==
			(mem_props))
		{
			type_index = i;
			break;
		}
	}
	if (type_index == UINT32_MAX)
		LOG_ERROR("Memory index Invalid");
	return type_index;
}

auto dazai_engine::renderer::copy_to_buffer(buffer* buffer, void* data, uint32_t size) -> void
{
	if (size >= buffer->size)
	{
		LOG_ERROR("Buffer size is greater than size");
		return;
	}
	if (buffer->data)
	{
		memcpy(buffer->data, data, size);
	}
	else
	{
		LOG_ERROR("Buffer data is null");
	}
}


