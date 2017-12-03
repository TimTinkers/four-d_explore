#ifndef APP_H_
#define APP_H_

// Imports.
#include <memory>
#include "misc/window.h"
#include "wrappers/instance.h"
#include "wrappers/queue.h"
#include "wrappers/rendering_surface.h"
#include "wrappers/swapchain.h"
#include "misc\time.h"

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
  Anvil::Time m_time;
  uint32_t n_last_semaphore_used_;
  const uint32_t n_swapchain_images_;
  VkDeviceSize ub_data_size_per_swapchain_image_;
  VkDeviceSize comp_per_swap_;

  // Sine data.
  std::shared_ptr<Anvil::Buffer> m_sine_color_buffer_ptr;        /* N_SINE_PAIRS * 2 * vec2; data stored as R8G8_UNORM */
  VkDeviceSize                   m_sine_color_buffer_size;
  std::shared_ptr<Anvil::Buffer> m_sine_data_buffer_ptr;
  std::vector<VkDeviceSize>      m_sine_data_buffer_offsets;
  VkDeviceSize                   m_sine_data_buffer_size;
  std::shared_ptr<Anvil::Buffer> m_sine_props_data_buffer_ptr;
  VkDeviceSize                   m_sine_props_data_buffer_size_per_swapchain_image;

  // Boilerplate initialization.
  void init_vulkan();
  void init_window();
  void init_swapchain();

  // Buffer initialization with helpers.
  std::shared_ptr<Anvil::Buffer> data_buffer_ptr_;
  std::shared_ptr<Anvil::Buffer> mesh_data_buffer_ptr_;
  std::shared_ptr<Anvil::Buffer> comp_data_buffer_ptr_;
  void init_buffers();
  const unsigned char* get_mesh_data() const;
  void get_buffer_memory_offsets(uint32_t  n_sine_pair,
	  uint32_t* out_opt_sine1SB_offset_ptr,
	  uint32_t* out_opt_sine2SB_offset_ptr,
	  uint32_t* out_opt_offset_data_offset_ptr = nullptr);

  // Descriptor set group initialization with helpers.
  std::shared_ptr<Anvil::DescriptorSetGroup> dsg_ptr_;
  std::shared_ptr<Anvil::DescriptorSetGroup> compute_dsg_ptr_;
  std::shared_ptr<Anvil::Buffer> m_sine_offset_data_buffer_ptr;
  std::vector<VkDeviceSize> m_sine_offset_data_buffer_offsets;
  VkDeviceSize m_sine_offset_data_buffer_size;
  void init_dsgs();

  // Image initialization.
  void init_images();

  // Semaphore handling and initialization with helpers.
  std::vector<std::shared_ptr<Anvil::Semaphore>> frame_signal_semaphores_;
  std::vector<std::shared_ptr<Anvil::Semaphore>> frame_wait_semaphores_;
  void init_semaphores();

  // Shader initialization and supporting helpers.
  std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> cs_ptr_;
  std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> fs_ptr_;
  std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> vs_ptr_;
  void init_shaders();

  // Compute pipeline initialization and helpers.
  Anvil::ComputePipelineID compute_pipeline_id_;
  void init_compute_pipelines();

  // Frame buffer initialization with helpers.
  std::shared_ptr<Anvil::Framebuffer> fbos_[N_SWAPCHAIN_IMAGES];
  std::shared_ptr<Anvil::Image> m_depth_images[N_SWAPCHAIN_IMAGES];
  std::shared_ptr<Anvil::ImageView> m_depth_image_views[N_SWAPCHAIN_IMAGES];
  void init_framebuffers();

  // Graphics pipeline initialization and helpers.
  std::shared_ptr<Anvil::RenderPass> renderpass_ptr_;
  Anvil::GraphicsPipelineID pipeline_id_;
  void init_gfx_pipelines();

  // Command buffer initialization and helpers.
  std::shared_ptr<Anvil::PrimaryCommandBuffer>
      command_buffers_[N_SWAPCHAIN_IMAGES];
  void get_luminance_data(std::shared_ptr<float>* out_result_ptr,
                          uint32_t* out_result_size_ptr) const;
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
