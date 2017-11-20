#ifndef APP_H_
#define APP_H_

// Imports.
#include <memory>
#include "misc/window.h"
#include "wrappers/instance.h"
#include "wrappers/queue.h"
#include "wrappers/rendering_surface.h"
#include "wrappers/swapchain.h"

// Global variables.
#define N_SWAPCHAIN_IMAGES 3

class App {

	public:

		// App boilerplate.
		App();
		void init();
		void run();

	private:
  
		// Field variables.
		std::weak_ptr<Anvil::SGPUDevice> device_ptr_;
		std::shared_ptr<Anvil::Instance> instance_ptr_;
		std::weak_ptr<Anvil::PhysicalDevice> physical_device_ptr_;
		std::shared_ptr<Anvil::Queue> present_queue_ptr_;
		std::shared_ptr<Anvil::RenderingSurface> rendering_surface_ptr_;
		std::shared_ptr<Anvil::Swapchain> swapchain_ptr_;
		std::shared_ptr<Anvil::Window> window_ptr_;

		// App intiialization: some values are given defaults when the app is created.
		uint32_t m_n_last_semaphore_used;
		const uint32_t n_swapchain_images_;
		VkDeviceSize m_ub_data_size_per_swapchain_image;

		// Boilerplate initialization.
		void init_vulkan();
		void init_window();
		void init_swapchain();

		// Buffer initialization with helpers.
		std::shared_ptr<Anvil::Buffer> data_buffer_ptr_;
		std::shared_ptr<Anvil::Buffer> mesh_data_buffer_ptr_;
		void init_buffers();
		const unsigned char* get_mesh_data() const;

		// Descriptor set group initialization with helpers.
		std::shared_ptr<Anvil::DescriptorSetGroup> dsg_ptr_;
		void init_dsgs();

		// Frame buffer initialization with helpers.
		std::shared_ptr<Anvil::Framebuffer> m_fbos[N_SWAPCHAIN_IMAGES];
		void init_framebuffers();

		// Semaphore handling and initialization with helpers.
		std::vector<std::shared_ptr<Anvil::Semaphore>> m_frame_signal_semaphores;
		std::vector<std::shared_ptr<Anvil::Semaphore>> m_frame_wait_semaphores;
		void init_semaphores();

		// Shader initialization and supporting helpers.
		std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> m_fs_ptr;
		std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> m_vs_ptr;
		void init_shaders();

		// Graphics pipeline initialization and helpers.
		std::shared_ptr<Anvil::RenderPass> m_renderpass_ptr;
		Anvil::GraphicsPipelineID m_pipeline_id;
		void init_gfx_pipelines();

		// Command buffer initialization and helpers.
		std::shared_ptr<Anvil::PrimaryCommandBuffer> m_command_buffers[N_SWAPCHAIN_IMAGES];
		void get_luminance_data(std::shared_ptr<float>* out_result_ptr, uint32_t * out_result_size_ptr) const;
		void init_command_buffers();

		// Frame validation.
		static VkBool32 on_validation_callback(VkDebugReportFlagsEXT message_flags,
												VkDebugReportObjectTypeEXT object_type,
												const char* layer_prefix,
												const char* message, void* user_arg);

		// Frame rendering.
		void update_data_ub_contents(uint32_t in_n_swapchain_image);
		static void draw_frame(void* app_raw_ptr);
};

#endif  // APP_H_
