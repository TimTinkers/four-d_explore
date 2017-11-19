#ifndef APP_H_
#define APP_H_

#include <memory>

#include "misc/window.h"
#include "wrappers/instance.h"
#include "wrappers/queue.h"
#include "wrappers/rendering_surface.h"
#include "wrappers/swapchain.h"

#define N_SWAPCHAIN_IMAGES 3

class App {
 public:
  App();
  void init();
  void run();

 private:
  void init_vulkan();
  void init_window();
  void init_swapchain();
  static void draw_frame (void* app_raw_ptr);
  static VkBool32 on_validation_callback(VkDebugReportFlagsEXT message_flags,
                                         VkDebugReportObjectTypeEXT object_type,
                                         const char* layer_prefix,
                                         const char* message, void* user_arg);

  std::weak_ptr<Anvil::SGPUDevice> device_ptr_;
  std::shared_ptr<Anvil::Instance> instance_ptr_;
  std::weak_ptr<Anvil::PhysicalDevice> physical_device_ptr_;
  std::shared_ptr<Anvil::Window> window_ptr_;
  std::shared_ptr<Anvil::RenderingSurface> rendering_surface_ptr_;
  std::shared_ptr<Anvil::Swapchain> swapchain_ptr_;
  std::shared_ptr<Anvil::Queue> present_queue_ptr_;

  const uint32_t n_swapchain_images_;

};

#endif  // APP_H_
