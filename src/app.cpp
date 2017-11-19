// #define ENABLE_VALIDATION

#include "app.h"

#include "misc/window_factory.h"
#include "wrappers/device.h"

#define APP_NAME "Four dimensional exploration"
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

App::App() : n_swapchain_images_(N_SWAPCHAIN_IMAGES) {}

void App::init() {
  init_vulkan();
  init_window();
}

void App::init_vulkan() {
  instance_ptr_ = Anvil::Instance::create(APP_NAME, APP_NAME,
#ifdef ENABLE_VALIDATION
                                          on_validation_callback,
#else
                                          nullptr,
#endif
                                          nullptr);
  physical_device_ptr_ = instance_ptr_->get_physical_device(0);
  device_ptr_ = Anvil::SGPUDevice::create(
      physical_device_ptr_, Anvil::DeviceExtensionConfiguration(),
      std::vector<std::string>(), false, false);
}

void App::init_window() {
  const Anvil::WindowPlatform platform = Anvil::WINDOW_PLATFORM_XCB;
  window_ptr_ = Anvil::WindowFactory::create_window(
      platform, APP_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, draw_frame, this);
}

void App::init_swapchain() {
  std::shared_ptr<Anvil::SGPUDevice> device_locked_ptr(device_ptr_);
  rendering_surface_ptr_ =
      Anvil::RenderingSurface::create(instance_ptr_, device_ptr_, window_ptr_);

  rendering_surface_ptr_->set_name("Main rendering surface");

  swapchain_ptr_ = device_locked_ptr->create_swapchain(
      rendering_surface_ptr_, window_ptr_, VK_FORMAT_B8G8R8A8_UNORM,
      VK_PRESENT_MODE_FIFO_KHR, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      n_swapchain_images_);

  swapchain_ptr_->set_name("Main swapchain");

  const std::vector<uint32_t>* present_queue_fams_ptr = nullptr;

  if (!rendering_surface_ptr_->get_queue_families_with_present_support(
          device_locked_ptr->get_physical_device(), &present_queue_fams_ptr)) {
    anvil_assert_fail();
  }

  present_queue_ptr_ =
      device_locked_ptr->get_queue(present_queue_fams_ptr->at(0), 0);
}

void App::draw_frame(void* app_raw_ptr) {
  // TODO
  return;
}

void App::run() {
  return;
}

VkBool32 App::on_validation_callback(VkDebugReportFlagsEXT message_flags,
                                     VkDebugReportObjectTypeEXT object_type,
                                     const char* layer_prefi,
                                     const char* message, void* user_arg) {
  if ((message_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0) {
    fprintf(stderr, "[!] %s\n", message);
  }
  return false;
}
