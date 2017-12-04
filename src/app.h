#ifndef APP_H_
#define APP_H_

// Imports.
#include <chrono>
#include <memory>
#include "misc/window.h"
#include "wrappers/instance.h"
#include "wrappers/queue.h"
#include "wrappers/rendering_surface.h"
#include "wrappers/swapchain.h"
#include "misc/time.h"

#include "camera.h"
#include "Window.h"

#define N_SWAPCHAIN_IMAGES (3)


class App {
public:
	/* Public functions */
	App();
	~App();

	void init();
	void run();

private:
	/* Private functions */
	App(const App&);

	void deinit();
	void init_buffers();
	void init_command_buffers();
	void init_compute_pipelines();
	void init_dsgs();
	void init_events();
	void init_framebuffers();
	void init_images();
	void init_gfx_pipelines();
	void init_semaphores();
	void init_shaders();
	void init_swapchain();
	void init_window();
	void init_vulkan();

	static void     draw_frame(void*                      app_raw_ptr);
	static VkBool32 on_validation_callback(VkDebugReportFlagsEXT      message_flags,
		VkDebugReportObjectTypeEXT object_type,
		const char*                layer_prefix,
		const char*                message,
		void*                      user_arg);

	/* Private variables */
	std::weak_ptr<Anvil::SGPUDevice>         m_device_ptr;
	std::shared_ptr<Anvil::Instance>         m_instance_ptr;
	std::weak_ptr<Anvil::PhysicalDevice>     m_physical_device_ptr;
	std::shared_ptr<Anvil::Queue>            m_present_queue_ptr;
	std::shared_ptr<Anvil::RenderingSurface> m_rendering_surface_ptr;
	std::shared_ptr<Anvil::Swapchain>        m_swapchain_ptr;
	Anvil::Time                              m_time;
	std::shared_ptr<Anvil::Window>           m_window_ptr;

	// Create descriptor groups for the compute pipeline.
	std::shared_ptr<Anvil::DescriptorSetGroup> m_consumer_dsg_ptr;
	std::shared_ptr<Anvil::DescriptorSetGroup> computeShaderDescriptorGroupPointer;

	std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> m_consumer_fs_ptr;
	std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> m_consumer_vs_ptr;
	std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> m_producer_cs_ptr;

	Anvil::GraphicsPipelineID          renderPipelineID;
	std::shared_ptr<Anvil::RenderPass> m_consumer_render_pass_ptr;
	Anvil::ComputePipelineID           computePipelineID;

	std::shared_ptr<Anvil::PrimaryCommandBuffer> m_command_buffers[N_SWAPCHAIN_IMAGES];
	std::shared_ptr<Anvil::Image>                m_depth_images[N_SWAPCHAIN_IMAGES];
	std::shared_ptr<Anvil::ImageView>            m_depth_image_views[N_SWAPCHAIN_IMAGES];
	std::shared_ptr<Anvil::Framebuffer>          m_fbos[N_SWAPCHAIN_IMAGES];
	
	// Create a pointer to a buffer for sending time data to the compute shader layout uniform.
	VkDeviceSize timeUniformSizePerSwapchain;
	std::shared_ptr<Anvil::Buffer> timeUniformPointer;

	// NEW: Cube.
	// Create a pointer to a buffer for storing the output cube vertices.
	VkDeviceSize outputCubeVerticesBufferSize;
	std::vector<VkDeviceSize> outputCubeVerticesBufferSizes;
	std::shared_ptr<Anvil::Buffer> outputCubeVerticesBufferPointer;

	// Create a pointer to a buffer for sending input cube vertices to the compute shader buffer.
	VkDeviceSize totalInputCubeBufferSize;
	std::vector<VkDeviceSize> inputCubeElementOffsets;
	std::shared_ptr<Anvil::Buffer> inputCubeBufferPointer;

	uint32_t       m_n_last_semaphore_used;
	const uint32_t m_n_swapchain_images;

	std::vector<std::shared_ptr<Anvil::Semaphore> > m_frame_signal_semaphores;
	std::vector<std::shared_ptr<Anvil::Semaphore> > m_frame_wait_semaphores;
};

#endif  // APP_H_