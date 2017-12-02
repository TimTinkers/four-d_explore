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

#include "camera.h"
#include "Window.h"

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
  uint32_t n_last_semaphore_used_;
  const uint32_t n_swapchain_images_;
  VkDeviceSize ub_data_size_per_swapchain_image_;
  Camera camera_;

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
  std::shared_ptr<Anvil::Framebuffer> fbos_[N_SWAPCHAIN_IMAGES];
  void init_framebuffers();

  // Semaphore handling and initialization with helpers.
  std::vector<std::shared_ptr<Anvil::Semaphore>> frame_signal_semaphores_;
  std::vector<std::shared_ptr<Anvil::Semaphore>> frame_wait_semaphores_;
  void init_semaphores();

  // Shader initialization and supporting helpers.
  std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> fs_ptr_;
  std::shared_ptr<Anvil::ShaderModuleStageEntryPoint> vs_ptr_;
  void init_shaders();

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

  // Keyboard.
  void init_camera();
  void handle_keys();

  VkSurfaceKHR surface_;

  std::chrono::time_point<std::chrono::steady_clock> prev_time;
};

#endif  // APP_H_
