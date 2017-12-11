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
#include "terrain.h"
#include "Window.h"

// Global variables.
#define N_SWAPCHAIN_IMAGES 3

class App {
 public:
  // App boilerplate.
  App(std::vector<Terrain::Block*> blocks);
  void init();
  void run();

  void ToggleRenderMode();

 private:
	 std::vector<Terrain::Block*> blocks_;
	 void init_meshes();

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

  Camera camera_;

  // Boilerplate initialization.
  void init_vulkan();
  void init_window();
  void init_swapchain();

  // Buffer initialization with helpers.
  std::shared_ptr<Anvil::Buffer> data_buffer_ptr_;
  std::shared_ptr<Anvil::Buffer> mesh_data_buffer_ptr_;
  std::shared_ptr<Anvil::Buffer> comp_data_buffer_ptr_;
  void init_buffers();

  // Descriptor set group initialization with helpers.
  std::shared_ptr<Anvil::DescriptorSetGroup> dsg_ptr_;
  std::shared_ptr<Anvil::DescriptorSetGroup> compute_dsg_ptr_;
  std::shared_ptr<Anvil::DescriptorSetGroup> axis_dsg_ptr_;
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
  std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> ge_ptr_;
  std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> vs_axis_ptr_;
  void init_shaders();

  // Compute pipeline initialization and helpers.
  Anvil::ComputePipelineID compute_pipeline_id_;
  void init_compute_pipelines();

  // Frame buffer initialization with helpers.
  std::shared_ptr<Anvil::Framebuffer> fbos_[N_SWAPCHAIN_IMAGES];
  std::shared_ptr<Anvil::ImageView> depth_image_views_[N_SWAPCHAIN_IMAGES];
  void init_framebuffers();

  // Graphics pipeline initialization and helpers.
  std::shared_ptr<Anvil::RenderPass> renderpass_ptr_;
  Anvil::GraphicsPipelineID pipeline_id_;
  void init_gfx_pipelines();
  std::shared_ptr<Anvil::Image> depth_images_[N_SWAPCHAIN_IMAGES];
  std::shared_ptr<Anvil::RenderPass> axis_renderpass_ptr_;
  Anvil::GraphicsPipelineID axis_pipeline_id_;

  // Command buffer initialization and helpers.
  std::shared_ptr<Anvil::PrimaryCommandBuffer> command_buffers_[N_SWAPCHAIN_IMAGES];
  void init_command_buffers();

  // Frame validation.
  static VkBool32 on_validation_callback(VkDebugReportFlagsEXT message_flags,
                                         VkDebugReportObjectTypeEXT object_type,
                                         const char* layer_prefix,
                                         const char* message, void* user_arg);

  // Frame rendering.
  static void draw_frame(void* app_raw_ptr);

  // Input.
  void init_camera();
  void handle_keys();

  // Create a pointer to a buffer for storing the output cube vertices.
  VkDeviceSize outputCubeVerticesBufferSize_;
  std::vector<VkDeviceSize> outputCubeVerticesBufferSizes_;
  std::shared_ptr<Anvil::Buffer> outputCubeVerticesBufferPointer_;

  // Create a pointer to a buffer for sending input cube vertices to the compute
  // shader buffer.
  VkDeviceSize totalInputCubeBufferSize_;
  std::vector<VkDeviceSize> inputCubeElementOffsets_;
  std::shared_ptr<Anvil::Buffer> inputCubeBufferPointer_;

  VkDeviceSize mat5UniformSizePerSwapchain;
  std::shared_ptr<Anvil::Buffer> viewProjUniformPointer;
  std::shared_ptr<Anvil::Buffer> viewMatrixUniformPointer;

  VkSurfaceKHR surface_;

  std::chrono::time_point<std::chrono::steady_clock> prev_time;
};

#endif  // APP_H_
