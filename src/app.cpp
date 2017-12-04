// Imports.
#include "app.h"

#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>

#include "config.h"
#include "misc/glsl_to_spirv.h"
#include "misc/io.h"
#include "misc/memory_allocator.h"
#include "misc/object_tracker.h"
#include "misc/time.h"
#include "misc/window_factory.h"
#include "wrappers/buffer.h"
#include "wrappers/command_buffer.h"
#include "wrappers/command_pool.h"
#include "wrappers/descriptor_set_group.h"
#include "wrappers/descriptor_set_layout.h"
#include "wrappers/device.h"
#include "wrappers/event.h"
#include "wrappers/compute_pipeline_manager.h"
#include "wrappers/graphics_pipeline_manager.h"
#include "wrappers/framebuffer.h"
#include "wrappers/image.h"
#include "wrappers/image_view.h"
#include "wrappers/instance.h"
#include "wrappers/physical_device.h"
#include "wrappers/rendering_surface.h"
#include "wrappers/query_pool.h"
#include "wrappers/render_pass.h"
#include "wrappers/semaphore.h"
#include "wrappers/shader_module.h"

#include "wrappers/swapchain.h"
#include "vulkan/vulkan.h"

#include "matrix.h"
#include "callback.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#else
#define GLFW_EXPOSE_NATIVE_X11
#include <X11/Xlib-xcb.h>
#endif
#include "GLFW/glfw3native.h"

/* Sanity checks */
#if defined(_WIN32)
#if !defined(ANVIL_INCLUDE_WIN3264_WINDOW_SYSTEM_SUPPORT) && !defined(ENABLE_OFFSCREEN_RENDERING)
#error Anvil has not been built with Win32/64 window system support. The application can only be built in offscreen rendering mode.
#endif
#else
#if !defined(ANVIL_INCLUDE_XCB_WINDOW_SYSTEM_SUPPORT) && !defined(ENABLE_OFFSCREEN_RENDERING)
#error Anvil has not been built with XCB window system support. The application can only be built in offscreen rendering mode.
#endif
#endif

/* Low-level #defines follow.. */
#define APP_NAME "Dynamic buffers example"

#define WINDOW_WIDTH  (1280)
#define WINDOW_HEIGHT (720)

/* When offscreen rendering is enabled, N_FRAMES_TO_RENDER tells how many frames should be
* rendered before leaving */
#define N_FRAMES_TO_RENDER (8)

App::App()
	:m_n_last_semaphore_used(0),
	m_n_swapchain_images(N_SWAPCHAIN_IMAGES) {
	// ..
}

App::~App() {
	deinit();
}

void App::deinit() {
	vkDeviceWaitIdle(m_device_ptr.lock()->get_device_vk());

	m_frame_signal_semaphores.clear();
	m_frame_wait_semaphores.clear();

	for (uint32_t n_cmd_buffer = 0;	
		n_cmd_buffer < sizeof(m_command_buffers) / sizeof(m_command_buffers[0]);
		++n_cmd_buffer) {
		m_command_buffers[n_cmd_buffer] = nullptr;
	}

	for (uint32_t n_depth_image = 0;
		n_depth_image < sizeof(m_depth_images) / sizeof(m_depth_images[0]);
		++n_depth_image) {
		m_depth_images[n_depth_image] = nullptr;
	}

	for (uint32_t n_depth_image_view = 0;
		n_depth_image_view < sizeof(m_depth_image_views) / sizeof(m_depth_image_views[0]);
		++n_depth_image_view) {
		m_depth_image_views[n_depth_image_view] = nullptr;
	}

	for (uint32_t n_fbo = 0;
		n_fbo < sizeof(m_fbos) / sizeof(m_fbos[0]);
		++n_fbo) {
		m_fbos[n_fbo] = nullptr;
	}

	renderDescriptorGroupPointer.reset();
	m_consumer_fs_ptr.reset();
	m_consumer_render_pass_ptr.reset();
	m_consumer_vs_ptr.reset();
	m_producer_cs_ptr.reset();
	computeShaderDescriptorGroupPointer.reset();

	timeUniformPointer.reset();

	// Reset the cube buffers.
	inputCubeBufferPointer.reset();
	outputCubeVerticesBufferPointer.reset();

	m_present_queue_ptr.reset();
	m_rendering_surface_ptr.reset();
	m_swapchain_ptr.reset();
	m_window_ptr.reset();

	m_device_ptr.lock()->destroy();
	m_device_ptr.reset();

	m_instance_ptr->destroy();
	m_instance_ptr.reset();
}

void App::draw_frame(void* app_raw_ptr) {
	App* app_ptr = static_cast<App*>(app_raw_ptr);
	std::shared_ptr<Anvil::Semaphore> curr_frame_signal_semaphore_ptr;
	std::shared_ptr<Anvil::Semaphore> curr_frame_wait_semaphore_ptr;
	std::shared_ptr<Anvil::SGPUDevice> device_locked_ptr = app_ptr->m_device_ptr.lock();
	static uint32_t n_frames_rendered = 0;
	uint32_t n_swapchain_image;
	std::shared_ptr<Anvil::Semaphore>  present_wait_semaphore_ptr;
	const VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	// Determine the signal + wait semaphores to use for drawing this frame.
	app_ptr->m_n_last_semaphore_used = (app_ptr->m_n_last_semaphore_used + 1) 
		% app_ptr->m_n_swapchain_images;

	curr_frame_signal_semaphore_ptr = app_ptr->
		m_frame_signal_semaphores[app_ptr->m_n_last_semaphore_used];
	curr_frame_wait_semaphore_ptr = app_ptr->
		m_frame_wait_semaphores[app_ptr->m_n_last_semaphore_used];

	present_wait_semaphore_ptr = curr_frame_signal_semaphore_ptr;

	// Determine the semaphore which the swapchain image.
	n_swapchain_image = app_ptr->m_swapchain_ptr->
		acquire_image(curr_frame_wait_semaphore_ptr, true);

	// Update time value, used by the generator compute shader.
	const uint64_t time_msec = app_ptr->m_time.get_time_in_msec();
	const float t = time_msec / 1000.0f;
	app_ptr->timeUniformPointer->write(
		app_ptr->timeUniformSizePerSwapchain * n_swapchain_image,	// Offset.
		sizeof(float),	// Size.
		&t);

	/* Submit jobs to relevant queues and make sure they are correctly synchronized */
	device_locked_ptr->get_universal_queue(0)
		->submit_command_buffer_with_signal_wait_semaphores(
			app_ptr->m_command_buffers[n_swapchain_image],
		1, /* n_semaphores_to_signal */
		&curr_frame_signal_semaphore_ptr,
		1, /* n_semaphores_to_wait_on */
		&curr_frame_wait_semaphore_ptr,
		&wait_stage_mask,
		false, /* should_block */
		nullptr);

	app_ptr->m_present_queue_ptr->present(app_ptr->m_swapchain_ptr,
		n_swapchain_image,
		1, /* n_wait_semaphores */
		&present_wait_semaphore_ptr);

	++n_frames_rendered;

#if defined(ENABLE_OFFSCREEN_RENDERING)
	{
		if (n_frames_rendered >= N_FRAMES_TO_RENDER) {
			m_window_ptr->close();
		}
	}
#endif
}

void App::init() {
	printf("i1\n");
	init_vulkan();
	printf("i2\n");
	init_window();
	printf("i3\n");
	init_swapchain();

	printf("i4\n");
	init_buffers();
	printf("i5\n");
	init_dsgs();
	printf("i6\n");
	init_images();
	printf("i7\n");
	init_semaphores();
	printf("i8\n");
	init_shaders();

	printf("i9\n");
	init_compute_pipelines();
	printf("i10\n");
	init_framebuffers();
	printf("i11\n");
	init_gfx_pipelines();
	printf("i12\n");
	init_command_buffers();
	printf("i13\n");
}

/*
 *	BUFFER INITIALIZATION.
 *	Initialize the various data buffers for use in the compute shader.
 */
void App::init_buffers() {

	// Setup the memory allocator to begin initializing data buffers.
	std::shared_ptr<Anvil::MemoryAllocator> memory_allocator_ptr;
	std::shared_ptr<Anvil::PhysicalDevice> physical_device_locked_ptr(m_physical_device_ptr);
	const VkDeviceSize sb_data_alignment_requirement = physical_device_locked_ptr->
		get_device_properties().limits.minStorageBufferOffsetAlignment;
	memory_allocator_ptr = Anvil::MemoryAllocator::create_oneshot(m_device_ptr);

	// NEW: cube.
	// Figure out what size is needed for the input buffer of cube vertices.
	totalInputCubeBufferSize = 0;
	for (uint32_t vertexIndex = 0; vertexIndex < 8; ++vertexIndex) {

		// Store current data offset.
		anvil_assert((totalInputCubeBufferSize % sb_data_alignment_requirement) == 0);
		inputCubeElementOffsets.push_back(totalInputCubeBufferSize);

		// Account for space necessary to hold a vec4 and any padding required to meet the alignment requirement.
		totalInputCubeBufferSize += (sizeof(float) * 4);
		totalInputCubeBufferSize += (sb_data_alignment_requirement - totalInputCubeBufferSize
			% sb_data_alignment_requirement) % sb_data_alignment_requirement;
	}

	// Create the layout buffer for storing the input cube vertices.
	inputCubeBufferPointer = Anvil::Buffer::create_nonsparse(m_device_ptr,
		totalInputCubeBufferSize,
		Anvil::QUEUE_FAMILY_COMPUTE_BIT | Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
		VK_SHARING_MODE_CONCURRENT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	inputCubeBufferPointer->set_name("Cube input vertices");
	memory_allocator_ptr->add_buffer(inputCubeBufferPointer, 0);

	// Prepare the actual values of input cube vertices.
	std::vector<glm::vec4> vertexData = { 
		glm::vec4(0, 0, 1, 1), glm::vec4(0.5, 0, 1, 1), glm::vec4(0.5, 0.5, 1, 1), glm::vec4(0, 0.5, 1, 1),
		glm::vec4(0.5, 0, 1, 1), glm::vec4(0.5, 0.5, 1, 1), glm::vec4(0, 0, 1, 1), glm::vec4(0, 0.5, 1, 1)
	};

	/*
		vec4(0, 0, 1, 1);
	} else if (current_invocation_id == 1) {
		outputMeshVertices.data[current_invocation_id] = vec4(0.5, 0, 1, 1);
	} else if (current_invocation_id == 2) {
		outputMeshVertices.data[current_invocation_id] = vec4(0.5, 0.5, 1, 1);
	} else if (current_invocation_id == 3) {
		outputMeshVertices.data[current_invocation_id] = vec4(0, 0.5, 1, 1);
	} else if (current_invocation_id == 4) {
		outputMeshVertices.data[current_invocation_id] = vec4(0.5, 0, 1, 1);
	} else if (current_invocation_id == 5) {
		outputMeshVertices.data[current_invocation_id] = vec4(0.5, 0.5, 1, 1);
	} else if (current_invocation_id == 6) {
		outputMeshVertices.data[current_invocation_id] = vec4(0, 0, 1, 1);
	} else if (current_invocation_id == 7) {
		outputMeshVertices.data[current_invocation_id] = vec4(0, 0.5, 1, 1);
	
	
	*/



	std::unique_ptr<char> inputCubeBufferValues;
	inputCubeBufferValues.reset(new char[static_cast<uintptr_t>(totalInputCubeBufferSize)]);
	for (uint32_t vertexIndex = 0; vertexIndex < 8; ++vertexIndex) {
		float* cubeVertexDataPointer = (float*)(inputCubeBufferValues.get()
			+ inputCubeElementOffsets[vertexIndex]);

		// Populate the component coordinates for each input vertex.
		glm::vec4 vertex = vertexData[vertexIndex];
		*cubeVertexDataPointer = vertex.x;
		cubeVertexDataPointer++;
		*cubeVertexDataPointer = vertex.y;
		cubeVertexDataPointer++;
		*cubeVertexDataPointer = vertex.z;
		cubeVertexDataPointer++;
		*cubeVertexDataPointer = vertex.w;
	}

	// Now prepare a memory block which is going to hold vertex data generated by the compute shader:
	outputCubeVerticesBufferSize = 0;
	for (unsigned int vertexIndex = 0; vertexIndex < 8; ++vertexIndex) {

		// Store current offset and account for space necessary to hold the generated sine data.
		outputCubeVerticesBufferSizes.push_back(outputCubeVerticesBufferSize);
		outputCubeVerticesBufferSize += (sizeof(float) * 4);

		// Account for space necessary to hold a vec4 for each point on the sine wave
		// and any padding required to meet the alignment requirement.
		outputCubeVerticesBufferSize += (sb_data_alignment_requirement - outputCubeVerticesBufferSize
			% sb_data_alignment_requirement) % sb_data_alignment_requirement;
		anvil_assert(outputCubeVerticesBufferSize % sb_data_alignment_requirement == 0);
	}

	// Allocate the memory for the buffer of output vertices.
	outputCubeVerticesBufferPointer = Anvil::Buffer::create_nonsparse(m_device_ptr,
		outputCubeVerticesBufferSize,
		Anvil::QUEUE_FAMILY_COMPUTE_BIT | Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
		VK_SHARING_MODE_CONCURRENT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	outputCubeVerticesBufferPointer->set_name("Cube output vertices");
	memory_allocator_ptr->add_buffer(outputCubeVerticesBufferPointer, 0);

	// END NEW.

	// We also need some space for a uniform block which is going to hold time info.
	const auto dynamic_ub_alignment_requirement = m_device_ptr.lock()->
		get_physical_device_properties().limits.minUniformBufferOffsetAlignment;
	const auto localTimeUniformSizePerSwapchain =
		Anvil::Utils::round_up(sizeof(float), dynamic_ub_alignment_requirement);
	const auto sine_props_data_buffer_size_total =
		localTimeUniformSizePerSwapchain * N_SWAPCHAIN_IMAGES;
	timeUniformSizePerSwapchain = localTimeUniformSizePerSwapchain;

	// Create the layout buffer for storing time in the compute shader.
	timeUniformPointer = Anvil::Buffer::create_nonsparse(
		m_device_ptr,
		sine_props_data_buffer_size_total,
		Anvil::QUEUE_FAMILY_COMPUTE_BIT | Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
		VK_SHARING_MODE_CONCURRENT,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	timeUniformPointer->set_name("Time data buffer");
	memory_allocator_ptr->add_buffer(timeUniformPointer,
		Anvil::MEMORY_FEATURE_FLAG_MAPPABLE);

	// NEW: cube.
	// Assign memory blocks to cube input vertices buffer and fill with values.
	inputCubeBufferPointer->write(0,
		inputCubeBufferPointer->get_size(),
		inputCubeBufferValues.get());
}

/*
*	COMMAND BUFFER INITIALIZATION.
*	Perform some actual drawing using the compute shader.
*/
void App::init_command_buffers() {

	// Boilerplate to prepare the graphics pipeline.
	std::shared_ptr<Anvil::SGPUDevice> device_locked_ptr(m_device_ptr);
	std::shared_ptr<Anvil::GraphicsPipelineManager> gfx_pipeline_manager_ptr(device_locked_ptr->get_graphics_pipeline_manager());
	const bool is_debug_marker_ext_present(device_locked_ptr->is_ext_debug_marker_extension_enabled());
	std::shared_ptr<Anvil::PipelineLayout> computePipelineLayoutPointer;
	VkImageSubresourceRange subresource_range;
	std::shared_ptr<Anvil::Queue> universal_queue_ptr(device_locked_ptr->get_universal_queue(0));

	computePipelineLayoutPointer = device_locked_ptr->get_compute_pipeline_manager()->get_compute_pipeline_layout(computePipelineID);

	subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_range.baseArrayLayer = 0;
	subresource_range.baseMipLevel = 0;
	subresource_range.layerCount = 1;
	subresource_range.levelCount = 1;

	// Set up rendering command buffers. We need one per swap-chain image.
	for (unsigned int n_current_swapchain_image = 0;
		n_current_swapchain_image < N_SWAPCHAIN_IMAGES;
		++n_current_swapchain_image) {

		std::shared_ptr<Anvil::PrimaryCommandBuffer> draw_cmd_buffer_ptr;
		draw_cmd_buffer_ptr = device_locked_ptr->get_command_pool(Anvil::QUEUE_FAMILY_TYPE_UNIVERSAL)->alloc_primary_level_command_buffer();

		// Start recording commands.
		draw_cmd_buffer_ptr->start_recording(false,	// One-time submit.
			true);	// Simultaneous use allowed.

		// Switch the swap-chain image layout to renderable.
		{
			Anvil::ImageBarrier image_barrier(0,	// Source access mask.
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,	// Destination access mask.
				false,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				universal_queue_ptr->get_queue_family_index(),
				universal_queue_ptr->get_queue_family_index(),
				m_swapchain_ptr->get_image(n_current_swapchain_image),
				subresource_range);

			draw_cmd_buffer_ptr->record_pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_FALSE,       // in_by_region
				0,              // in_memory_barrier_count
				nullptr,        // in_memory_barrier_ptrs
				0,              // in_buffer_memory_barrier_count
				nullptr,        // in_buffer_memory_barrier_ptrs
				1,              // in_image_memory_barrier_count
				&image_barrier);
		}

		// Invalidate the shader read cache for this CPU-written data.
		Anvil::BufferBarrier t_value_buffer_barrier = Anvil::BufferBarrier(
			VK_ACCESS_HOST_WRITE_BIT,
			VK_ACCESS_UNIFORM_READ_BIT,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			timeUniformPointer,
			n_current_swapchain_image * timeUniformSizePerSwapchain,
			sizeof(float));

		draw_cmd_buffer_ptr->record_pipeline_barrier(
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_FALSE,
			0,                       // in_memory_barrier_count
			nullptr,                 // in_memory_barriers_ptr
			1,                       // in_buffer_memory_barrier_count
			&t_value_buffer_barrier, // in_buffer_memory_barriers_ptr
			0,                       // in_image_memory_barrier_count
			nullptr);                // in_image_memory_barriers_ptr

		// Let's generate some sine offset data using our compute shader.
		draw_cmd_buffer_ptr->record_bind_pipeline(VK_PIPELINE_BIND_POINT_COMPUTE,
			computePipelineID);

		if (is_debug_marker_ext_present) {
			static const float region_color[4] = {
				0.0f,
				1.0f,
				0.0f,
				1.0f };
			draw_cmd_buffer_ptr->record_debug_marker_begin_EXT("Sine offset data computation",
				region_color);
		}

		printf("c0\n");
		std::shared_ptr<Anvil::DescriptorSet> producer_dses[] =
		{
			computeShaderDescriptorGroupPointer->get_descriptor_set(0),
			computeShaderDescriptorGroupPointer->get_descriptor_set(1)
		};

		printf("c0.1\n");
		static const uint32_t n_producer_dses = sizeof(producer_dses) / sizeof(producer_dses[0]);

		printf("c1 n_producer_dses: %d\n", n_producer_dses);
		draw_cmd_buffer_ptr->record_bind_descriptor_sets(VK_PIPELINE_BIND_POINT_COMPUTE,
			computePipelineLayoutPointer,
			0, /* firstSet */
			n_producer_dses,
			producer_dses,
			0,
			nullptr);

		draw_cmd_buffer_ptr->record_dispatch(2,  /* x */
			1,  /* y */
			1); /* z */

		if (is_debug_marker_ext_present) {
			draw_cmd_buffer_ptr->record_debug_marker_end_EXT();
		}
		printf("c2\n");

		// Now, use the generated data to draw stuff!
		VkClearValue clear_values[2];
		VkRect2D     render_area;

		clear_values[0].color.float32[0] = 0.25f;
		clear_values[0].color.float32[1] = 0.50f;
		clear_values[0].color.float32[2] = 0.75f;
		clear_values[0].color.float32[3] = 1.0f;
		clear_values[1].depthStencil.depth = 1.0f;

		render_area.extent.height = WINDOW_HEIGHT;
		render_area.extent.width = WINDOW_WIDTH;
		render_area.offset.x = 0;
		render_area.offset.y = 0;

		// NOTE: The render-pass switches the swap-chain image back to the presentable layout
		//      after the draw call finishes.
		draw_cmd_buffer_ptr->record_begin_render_pass(2, // n_clear_values
			clear_values,
			m_fbos[n_current_swapchain_image],
			render_area,
			m_consumer_render_pass_ptr,
			VK_SUBPASS_CONTENTS_INLINE);
		{
			const float max_line_width = m_device_ptr.lock()->get_physical_device_properties().limits.lineWidthRange[1];
			std::shared_ptr<Anvil::DescriptorSet> renderer_dses[] = {
				renderDescriptorGroupPointer->get_descriptor_set(0) };
			const uint32_t n_renderer_dses = sizeof(renderer_dses) / sizeof(renderer_dses[0]);

			std::shared_ptr<Anvil::PipelineLayout> renderer_pipeline_layout_ptr;

			renderer_pipeline_layout_ptr = gfx_pipeline_manager_ptr->get_graphics_pipeline_layout(renderPipelineID);

			draw_cmd_buffer_ptr->record_bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS,
				renderPipelineID);

			static const VkDeviceSize offsets = 0;
			draw_cmd_buffer_ptr->record_bind_vertex_buffers(0, // startBinding
				1, // bindingCount
				&inputCubeBufferPointer,
				&offsets);

			// Set line width.
			float lineWidth = 2;
			draw_cmd_buffer_ptr->record_set_line_width(lineWidth);
			printf("c4\n");

			draw_cmd_buffer_ptr->record_bind_descriptor_sets(VK_PIPELINE_BIND_POINT_GRAPHICS,
				renderer_pipeline_layout_ptr,
				0, /* firstSet */
				n_renderer_dses,
				renderer_dses,
				0,
				nullptr);

			draw_cmd_buffer_ptr->record_draw(8,
				1,                /* instanceCount */
				0,                /* firstVertex   */
				0); /* firstInstance */
		}
		draw_cmd_buffer_ptr->record_end_render_pass();
		printf("c5\n");

		// Close the recording process.
		draw_cmd_buffer_ptr->stop_recording();
		m_command_buffers[n_current_swapchain_image] = draw_cmd_buffer_ptr;
		printf("c6\n");
	}
}

void App::init_compute_pipelines() {
	std::shared_ptr<Anvil::SGPUDevice>             device_locked_ptr(m_device_ptr);
	std::shared_ptr<Anvil::ComputePipelineManager> compute_manager_ptr(device_locked_ptr->get_compute_pipeline_manager());
	bool                                           result;

	/* Create & configure the compute pipeline */
	result = compute_manager_ptr->add_regular_pipeline(false, /* disable_optimizations */
		false, /* allow_derivatives     */
		*m_producer_cs_ptr,
		&computePipelineID);
	anvil_assert(result);

	result = compute_manager_ptr->attach_push_constant_range_to_pipeline(computePipelineID,
		0,  /* offset */
		4,  /* size   */
		VK_SHADER_STAGE_COMPUTE_BIT);
	anvil_assert(result);

	result = compute_manager_ptr->set_pipeline_dsg(computePipelineID,
		computeShaderDescriptorGroupPointer);
	anvil_assert(result);

	result = compute_manager_ptr->bake();
	anvil_assert(result);
}

void App::init_dsgs() {
	/* Create the descriptor set layouts for the generator program. */
	computeShaderDescriptorGroupPointer = Anvil::DescriptorSetGroup::create(m_device_ptr,
		false, /* releaseable_sets */
		3      /* n_sets           */);

	computeShaderDescriptorGroupPointer->add_binding(0, /* n_set      */
		0, /* binding    */
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1, /* n_elements */
		VK_SHADER_STAGE_COMPUTE_BIT);

	printf("dsg1\n");
	computeShaderDescriptorGroupPointer->add_binding(1,	// Set.
		0,	// Binding.
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1, // n elements.
		VK_SHADER_STAGE_COMPUTE_BIT);

	printf("dsg3\n");
	computeShaderDescriptorGroupPointer->add_binding(1,	// Set.
		1,	// Binding.
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		1,	// n elements.
		VK_SHADER_STAGE_COMPUTE_BIT);

	// Bind to the compute shader a uniform layout for storing the current time.
	computeShaderDescriptorGroupPointer->set_binding_item(
		0,	// Set.
		0,	// Binding.
		Anvil::DescriptorSet::UniformBufferBindingElement(
			timeUniformPointer,
			0,	// Offset.
			timeUniformSizePerSwapchain));

	printf("dsg2\n");
	// NEW: cube
	// Bind to the compute shader a buffer for recording input cube vertices.
	computeShaderDescriptorGroupPointer->set_binding_item(
		1,	// Set.
		0,	// Binding.
		Anvil::DescriptorSet::UniformBufferBindingElement(
			inputCubeBufferPointer,
			0,	// Offset.
			sizeof(float) * 4 * 8));

	printf("dsg4\n");
	// Bind to the compute shader a buffer for recording the output cube vertices.
	computeShaderDescriptorGroupPointer->set_binding_item(
		1,	// Set.
		1,	// Binding.
		Anvil::DescriptorSet::StorageBufferBindingElement(
			outputCubeVerticesBufferPointer,
			0,	// Offset.
			sizeof(float) * 4 * 8));
	printf("dsg5\n");

	/* Set up the descriptor set layout for the renderer program.  */
	renderDescriptorGroupPointer = Anvil::DescriptorSetGroup::create(m_device_ptr,
		false, /* releaseable_sets */
		1      /* n_sets           */);

	renderDescriptorGroupPointer->add_binding(0, /* n_set      */
		0, /* binding    */
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		1, /* n_elements */
		VK_SHADER_STAGE_VERTEX_BIT);

	renderDescriptorGroupPointer->set_binding_item(0, /* n_set         */
		0, /* binding_index */
		Anvil::DescriptorSet::StorageBufferBindingElement(outputCubeVerticesBufferPointer,
			0, /* in_start_offset */
			sizeof(float) * 4 * 8));
}

void App::init_events() {
}

void App::init_framebuffers() {
	bool result;

	for (uint32_t n_swapchain_image = 0;
	n_swapchain_image < N_SWAPCHAIN_IMAGES;
		++n_swapchain_image) {
		std::shared_ptr<Anvil::Framebuffer> result_fb_ptr;

		result_fb_ptr = Anvil::Framebuffer::create(m_device_ptr,
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
			1); /* n_layers */

		result_fb_ptr->set_name_formatted("Framebuffer for swapchain image [%d]",
			n_swapchain_image);

		result = result_fb_ptr->add_attachment(m_swapchain_ptr->get_image_view(n_swapchain_image),
			nullptr); /* out_opt_attachment_id_ptr */
		anvil_assert(result);

		result = result_fb_ptr->add_attachment(m_depth_image_views[n_swapchain_image],
			nullptr); /* out_opt_attachment_id_ptr */

		m_fbos[n_swapchain_image] = result_fb_ptr;
	}
}

void App::init_gfx_pipelines() {
	std::shared_ptr<Anvil::SGPUDevice>              device_locked_ptr(m_device_ptr);
	std::shared_ptr<Anvil::GraphicsPipelineManager> gfx_manager_ptr(device_locked_ptr->get_graphics_pipeline_manager());
	bool                                            result;

	/* Create a renderpass instance */
#ifdef ENABLE_OFFSCREEN_RENDERING
	const VkImageLayout final_layout = VK_IMAGE_LAYOUT_GENERAL;
#else
	const VkImageLayout final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
#endif

	Anvil::RenderPassAttachmentID render_pass_color_attachment_id = -1;
	Anvil::RenderPassAttachmentID render_pass_depth_attachment_id = -1;
	Anvil::SubPassID              render_pass_subpass_id = -1;

	m_consumer_render_pass_ptr = Anvil::RenderPass::create(m_device_ptr,
		m_swapchain_ptr);

	m_consumer_render_pass_ptr->set_name("Consumer renderpass");

	result = m_consumer_render_pass_ptr->add_color_attachment(m_swapchain_ptr->get_image_format(),
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		final_layout,
		false, /* may_alias */
		&render_pass_color_attachment_id);
	anvil_assert(result);

	result = m_consumer_render_pass_ptr->add_depth_stencil_attachment(m_depth_images[0]->get_image_format(),
		m_depth_images[0]->get_image_sample_count(),
		VK_ATTACHMENT_LOAD_OP_CLEAR,                      /* depth_load_op    */
		VK_ATTACHMENT_STORE_OP_DONT_CARE,                 /* depth_store_op   */
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,                  /* stencil_load_op  */
		VK_ATTACHMENT_STORE_OP_DONT_CARE,                 /* stencil_store_op */
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, /* initial_layout   */
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, /* final_layout     */
		false,                                            /* may_alias        */
		&render_pass_depth_attachment_id);
	anvil_assert(result);

	result = m_consumer_render_pass_ptr->add_subpass(*m_consumer_fs_ptr,
		Anvil::ShaderModuleStageEntryPoint(), /* geometry_shader        */
		Anvil::ShaderModuleStageEntryPoint(), /* tess_control_shader    */
		Anvil::ShaderModuleStageEntryPoint(), /* tess_evaluation_shader */
		*m_consumer_vs_ptr,
		&render_pass_subpass_id);
	anvil_assert(result);

	result = m_consumer_render_pass_ptr->add_subpass_color_attachment(render_pass_subpass_id,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		render_pass_color_attachment_id,
		0,        /* location                      */
		nullptr); /* opt_attachment_resolve_id_ptr */
	result &= m_consumer_render_pass_ptr->add_subpass_depth_stencil_attachment(render_pass_subpass_id,
		render_pass_depth_attachment_id,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	anvil_assert(result);

	/* Set up the graphics pipeline for the main subpass */
	result = m_consumer_render_pass_ptr->get_subpass_graphics_pipeline_id(render_pass_subpass_id,
		&renderPipelineID);
	anvil_assert(result);

	gfx_manager_ptr->add_vertex_attribute(renderPipelineID,
		0, /* location */
		VK_FORMAT_R32G32B32A32_SFLOAT,
		0,                /* offset_in_bytes */
		sizeof(float) * 1, /* stride_in_bytes */
		VK_VERTEX_INPUT_RATE_INSTANCE);
	gfx_manager_ptr->set_pipeline_dsg(renderPipelineID,
		renderDescriptorGroupPointer);
	gfx_manager_ptr->set_input_assembly_properties(renderPipelineID,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
	gfx_manager_ptr->set_rasterization_properties(renderPipelineID,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_NONE,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		10.0f /* line_width */);
	gfx_manager_ptr->toggle_depth_test(renderPipelineID,
		true, /* should_enable */
		VK_COMPARE_OP_LESS_OR_EQUAL);
	gfx_manager_ptr->toggle_depth_writes(renderPipelineID,
		true); /* should_enable */
	gfx_manager_ptr->toggle_dynamic_states(renderPipelineID,
		true, /* should_enable */
		Anvil::GraphicsPipelineManager::DYNAMIC_STATE_LINE_WIDTH_BIT);
}

void App::init_images() {
	for (uint32_t n_depth_image = 0; n_depth_image < N_SWAPCHAIN_IMAGES; ++n_depth_image) {
		m_depth_images[n_depth_image] = Anvil::Image::create_nonsparse(m_device_ptr,
			VK_IMAGE_TYPE_2D,
			VK_FORMAT_D16_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
			1,                    /* in_base_mipmap_depth */
			1,                    /* in_n_layers          */
			VK_SAMPLE_COUNT_1_BIT,
			Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			false,                /* in_use_full_mipmap_chain */
			0,                    /* in_memory_features       */
			0,                    /* in_create_flags          */
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			nullptr);             /* in_mipmaps_ptr */

		m_depth_image_views[n_depth_image] = Anvil::ImageView::create_2D(m_device_ptr,
			m_depth_images[n_depth_image],
			0,                                              /* n_base_layer        */
			0,                                              /* n_base_mipmap_level */
			1,                                              /* n_mipmaps           */
			VK_IMAGE_ASPECT_DEPTH_BIT,
			m_depth_images[n_depth_image]->get_image_format(),
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY);

		m_depth_images[n_depth_image]->set_name_formatted("Depth image [%d]",
			n_depth_image);
		m_depth_image_views[n_depth_image]->set_name_formatted("Depth image view [%d]",
			n_depth_image);
	}
}

void App::init_semaphores() {
	for (uint32_t n_semaphore = 0;
	n_semaphore < m_n_swapchain_images;
		++n_semaphore) {
		std::shared_ptr<Anvil::Semaphore> new_signal_semaphore_ptr = Anvil::Semaphore::create(m_device_ptr);
		std::shared_ptr<Anvil::Semaphore> new_wait_semaphore_ptr = Anvil::Semaphore::create(m_device_ptr);

		new_signal_semaphore_ptr->set_name_formatted("Signal semaphore [%d]",
			n_semaphore);
		new_wait_semaphore_ptr->set_name_formatted("Wait semaphore [%d]",
			n_semaphore);

		m_frame_signal_semaphores.push_back(new_signal_semaphore_ptr);
		m_frame_wait_semaphores.push_back(new_wait_semaphore_ptr);
	}
}

void App::init_shaders() {

	// Read the compute shader from a separate file.
	// If running on Windows, assume it's Tim's machine and resolve this hacky path garbage.
#ifdef _WIN32
	std::ifstream infileComp{ "E:\\Penn 17 - 18\\CIS 565\\four-d_explore\\src\\shaders\\example.comp" };
#else
	std::ifstream infileComp{ "../src/shaders/example.comp" };
#endif
	std::string compute{ std::istreambuf_iterator<char>(infileComp), std::istreambuf_iterator<char>() };

	// Read the vertex shader from a separate file.
	// If running on Windows, assume it's Tim's machine and resolve this hacky path garbage.
#ifdef _WIN32
	std::ifstream infileVertex{ "E:\\Penn 17 - 18\\CIS 565\\four-d_explore\\src\\shaders\\example.vert" };
#else
	std::ifstream infileVertex{ "../src/shaders/example.vert" };
#endif
	std::string vertex{ std::istreambuf_iterator<char>(infileVertex), std::istreambuf_iterator<char>() };

	// Read the fragment shader from a separate file.
	// If running on Windows, assume it's Tim's machine and resolve this hacky path garbage.
#ifdef _WIN32
	std::ifstream infileFragment{ "E:\\Penn 17 - 18\\CIS 565\\four-d_explore\\src\\shaders\\example.frag" };
#else
	std::ifstream infileFragment{ "../src/shaders/example.frag" };
#endif
	std::string fragment{ std::istreambuf_iterator<char>(infileFragment), std::istreambuf_iterator<char>() };

	std::shared_ptr<Anvil::ShaderModule>               cs_module_ptr;
	std::shared_ptr<Anvil::GLSLShaderToSPIRVGenerator> cs_ptr;
	std::shared_ptr<Anvil::ShaderModule>               fs_module_ptr;
	std::shared_ptr<Anvil::GLSLShaderToSPIRVGenerator> fs_ptr;
	std::shared_ptr<Anvil::ShaderModule>               vs_module_ptr;
	std::shared_ptr<Anvil::GLSLShaderToSPIRVGenerator> vs_ptr;

	cs_ptr = Anvil::GLSLShaderToSPIRVGenerator::create(m_device_ptr,
		Anvil::GLSLShaderToSPIRVGenerator::MODE_USE_SPECIFIED_SOURCE,
		compute,
		Anvil::SHADER_STAGE_COMPUTE);
	vs_ptr = Anvil::GLSLShaderToSPIRVGenerator::create(m_device_ptr,
		Anvil::GLSLShaderToSPIRVGenerator::MODE_USE_SPECIFIED_SOURCE,
		vertex,
		Anvil::SHADER_STAGE_VERTEX);
	fs_ptr = Anvil::GLSLShaderToSPIRVGenerator::create(m_device_ptr,
		Anvil::GLSLShaderToSPIRVGenerator::MODE_USE_SPECIFIED_SOURCE,
		fragment,
		Anvil::SHADER_STAGE_FRAGMENT);

	/* Initialize the shader modules */
	cs_module_ptr = Anvil::ShaderModule::create_from_spirv_generator(m_device_ptr,
		cs_ptr);
	fs_module_ptr = Anvil::ShaderModule::create_from_spirv_generator(m_device_ptr,
		fs_ptr);
	vs_module_ptr = Anvil::ShaderModule::create_from_spirv_generator(m_device_ptr,
		vs_ptr);

	cs_module_ptr->set_name("Compute shader module");
	fs_module_ptr->set_name("Fragment shader module");
	vs_module_ptr->set_name("Vertex shader module");

	/* Prepare entrypoint descriptors. */
	m_producer_cs_ptr.reset(
		new Anvil::ShaderModuleStageEntryPoint("main",
			cs_module_ptr,
			Anvil::SHADER_STAGE_COMPUTE)
		);
	m_consumer_fs_ptr.reset(
		new Anvil::ShaderModuleStageEntryPoint("main",
			fs_module_ptr,
			Anvil::SHADER_STAGE_FRAGMENT)
		);
	m_consumer_vs_ptr.reset(
		new Anvil::ShaderModuleStageEntryPoint("main",
			vs_module_ptr,
			Anvil::SHADER_STAGE_VERTEX)
		);
}

void App::init_swapchain() {
	std::shared_ptr<Anvil::SGPUDevice> device_locked_ptr(m_device_ptr);

	m_rendering_surface_ptr = Anvil::RenderingSurface::create(m_instance_ptr,
		m_device_ptr,
		m_window_ptr);

	m_rendering_surface_ptr->set_name("Main rendering surface");


	m_swapchain_ptr = device_locked_ptr->create_swapchain(m_rendering_surface_ptr,
		m_window_ptr,
		VK_FORMAT_B8G8R8A8_UNORM,
		VK_PRESENT_MODE_FIFO_KHR,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		m_n_swapchain_images);

	m_swapchain_ptr->set_name("Main swapchain");

	/* Cache the queue we are going to use for presentation */
	const std::vector<uint32_t>* present_queue_fams_ptr = nullptr;

	if (!m_rendering_surface_ptr->get_queue_families_with_present_support(device_locked_ptr->get_physical_device(),
		&present_queue_fams_ptr)) {
		anvil_assert_fail();
	}

	m_present_queue_ptr = device_locked_ptr->get_queue(present_queue_fams_ptr->at(0),
		0); /* in_n_queue */
}

/*
WINDOW INITIALIZATION.
Initialize the window for displaying this app.
*/
void App::init_window() {
	InitializeWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME);

#ifdef _WIN32
	const Anvil::WindowPlatform platform = Anvil::WINDOW_PLATFORM_SYSTEM;
	WindowHandle handle = glfwGetWin32Window(GetGLFWWindow());
	void* xcb_ptr = nullptr;
#else
	const Anvil::WindowPlatform platform = Anvil::WINDOW_PLATFORM_XCB;
	WindowHandle handle = glfwGetX11Window(GetGLFWWindow());
	void* xcb_ptr = (void*)XGetXCBConnection(glfwGetX11Display());
#endif

	m_window_ptr = Anvil::WindowFactory::create_window(platform, handle, xcb_ptr);
}

void App::init_vulkan() {
	/* Create a Vulkan instance */
	m_instance_ptr = Anvil::Instance::create(APP_NAME,  /* app_name */
		APP_NAME,  /* engine_name */
#ifdef ENABLE_VALIDATION
		on_validation_callback,
#else
		nullptr,
#endif
		nullptr);

	m_physical_device_ptr = m_instance_ptr->get_physical_device(0);

	/* Create a Vulkan device */
	m_device_ptr = Anvil::SGPUDevice::create(m_physical_device_ptr,
		Anvil::DeviceExtensionConfiguration(),
		std::vector<std::string>(), /* layers                               */
		false,                      /* transient_command_buffer_allocs_only */
		false);                     /* support_resettable_command_buffers   */
}

VkBool32 App::on_validation_callback(VkDebugReportFlagsEXT message_flags,
	VkDebugReportObjectTypeEXT object_type,
	const char* layer_prefi,
	const char* message, void* user_arg) {
	// Display any detected error.
	if ((message_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0) {
		fprintf(stderr, "[!] %s\n", message);
	}
	return false;
}

void App::run() {
	while (!ShouldQuit()) {
		glfwPollEvents();
		draw_frame(this);
		//auto cur_time = std::chrono::steady_clock::now();
		//std::chrono::duration<double, std::milli> dif = cur_time - prev_time;
		//std::cout << dif.count() << "\n";
		//prev_time = cur_time;
		// handle_keys();
	}
	DestroyWindow();
}