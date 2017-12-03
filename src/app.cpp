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

// Debug flags.
// #define ENABLE_VALIDATION
// TODO: change this.

/** Total number of sines to draw. */
#define N_SINE_PAIRS (4)

/** Number of vertices to approximate sine shape with. */
#define N_VERTICES_PER_SINE (128)

// Constants.
#define APP_NAME "Four Dimensional Exploration"
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define GLFW

// Field variables.
// Mesh data.
const float g_mesh_data[] = {
    -1.0f, 1.0f,  0.0f, 1.0f, /* position */
    0.75f, 0.25f, 0.1f,       /* color    */
    -1.0f, -1.0f, 0.0f, 1.0f, /* position */
    0.25f, 0.75f, 0.2f,       /* color    */
    1.0f,  -1.0f, 0.0f, 1.0f, /* position */
    0.1f,  0.3f,  0.5f,       /* color    */
};
static const uint32_t g_mesh_data_color_start_offset = sizeof(float) * 4;
static const uint32_t g_mesh_data_color_stride = sizeof(float) * 7;
static const uint32_t g_mesh_data_n_vertices = 3;
static const uint32_t g_mesh_data_position_start_offset = 0;
static const uint32_t g_mesh_data_position_stride = sizeof(float) * 7;

// Count the number of example triangles.
const int N_TRIANGLES = 16;

/*
Create the app and assign default values to several field variables.
*/
App::App()
    : n_last_semaphore_used_(0),
      n_swapchain_images_(N_SWAPCHAIN_IMAGES),
      prev_time(std::chrono::steady_clock::now()) {}

/*
 This function initializes the app through a series of smaller initialization
 steps.
 The GPUOpen example project "PushConstants" was a starting point for this
 project.
 https://github.com/GPUOpen-LibrariesAndSDKs/Anvil/blob/master/examples/PushConstants
 */
void App::init() {
  init_vulkan();
  init_window();
  init_swapchain();
  printf("s1\n");
  init_buffers();
  printf("s2\n");
  init_dsgs();
  printf("s3\n");
  init_images();
  printf("s4\n");
  init_semaphores();
  printf("s5\n");
  init_shaders();
  printf("s6\n");

  init_compute_pipelines();
  printf("s7\n");
  init_framebuffers();
  printf("s8\n");
  init_gfx_pipelines();
  printf("s9\n");
  init_command_buffers();

  printf("s10\n");
  init_camera();
}

/*
  VULKAN INITIALIZATION.
  Initialize the Vulkan context to work with this app.
 */
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

  window_ptr_ = Anvil::WindowFactory::create_window(platform, handle, xcb_ptr);
}

/*
  SWAPCHAIN INITIALIZATION.
  Initialize the app's main swapchain.
 */
void App::init_swapchain() {
  std::shared_ptr<Anvil::SGPUDevice> device_locked_ptr(device_ptr_);
  rendering_surface_ptr_ =
      Anvil::RenderingSurface::create(instance_ptr_, device_ptr_, window_ptr_);
  //rendering_surface_ptr_->get_surface_ptr() = &surface_;

  rendering_surface_ptr_->set_name("Main rendering surface");

  swapchain_ptr_ = device_locked_ptr->create_swapchain(
      rendering_surface_ptr_, window_ptr_, VK_FORMAT_B8G8R8A8_UNORM,
      VK_PRESENT_MODE_FIFO_KHR, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      n_swapchain_images_);
  swapchain_ptr_->set_name("Main swapchain");

  /* Cache the queue we are going to use for presentation */
  const std::vector<uint32_t>* present_queue_fams_ptr = nullptr;
  if (!rendering_surface_ptr_->get_queue_families_with_present_support(
          device_locked_ptr->get_physical_device(), &present_queue_fams_ptr)) {
    anvil_assert_fail();
  }

  present_queue_ptr_ =
      device_locked_ptr->get_queue(present_queue_fams_ptr->at(0), 0);
}

/*
 BUFFER INITIALIZATION.
 Initialize the buffers for geometry in the scene.
 This section includes relevant helper functions.
 */

// Retrieve data on the scene mesh to display.
const unsigned char* App::get_mesh_data() const {
  return reinterpret_cast<const unsigned char*>(g_mesh_data);
}

// A helper to retrieve memory offsets for the sine buffer.
void App::get_buffer_memory_offsets(uint32_t  n_sine_pair,
	uint32_t* out_opt_sine1SB_offset_ptr,
	uint32_t* out_opt_sine2SB_offset_ptr,
	uint32_t* out_opt_offset_data_offset_ptr) {
	if (out_opt_sine1SB_offset_ptr != nullptr) {
		*out_opt_sine1SB_offset_ptr = static_cast<uint32_t>(
			m_sine_data_buffer_offsets[n_sine_pair * 2]);
	}

	if (out_opt_sine2SB_offset_ptr != nullptr) {
		*out_opt_sine2SB_offset_ptr = static_cast<uint32_t>(
			m_sine_data_buffer_offsets[n_sine_pair * 2 + 1]);
	}

	if (out_opt_offset_data_offset_ptr != nullptr) {
		const uint32_t sb_offset_alignment = static_cast<uint32_t>(
			physical_device_ptr_.lock()->get_device_properties().limits
			.minStorageBufferOffsetAlignment);

		*out_opt_offset_data_offset_ptr = Anvil::Utils::round_up(
			static_cast<uint32_t>(sizeof(float) * 2),
			sb_offset_alignment) * n_sine_pair;
	}
}

// Buffer initialization.
void App::init_buffers() {
/*
  const unsigned char* mesh_data = get_mesh_data();
  const uint32_t mesh_data_size = sizeof(g_mesh_data);
  const VkDeviceSize ub_data_size_per_swapchain_image =
      sizeof(int) * 4 +                 // frame index + padding             
      sizeof(float) * N_TRIANGLES * 4 + // position (vec2) + rotation (vec2)
      sizeof(float) * N_TRIANGLES +     // luminance                        
      sizeof(float) * N_TRIANGLES;      // size                             
  const auto ub_data_alignment_requirement =
      device_ptr_.lock()
          ->get_physical_device_properties()
          .limits.minUniformBufferOffsetAlignment;
  const auto ub_data_size_total =
      N_SWAPCHAIN_IMAGES *
      (Anvil::Utils::round_up(ub_data_size_per_swapchain_image,
                              ub_data_alignment_requirement));
  ub_data_size_per_swapchain_image_ = ub_data_size_total / N_SWAPCHAIN_IMAGES;
  */

	std::shared_ptr<Anvil::PhysicalDevice> physical_device_locked_ptr(
		physical_device_ptr_);
	const VkDeviceSize sb_data_alignment_requirement = physical_device_locked_ptr->
		get_device_properties().limits.minStorageBufferOffsetAlignment;
	std::unique_ptr<char> sine_offset_data_raw_buffer_ptr;

	/* Use a memory allocator to re-use memory blocks wherever possible */
  std::shared_ptr<Anvil::MemoryAllocator> allocator_ptr =
      Anvil::MemoryAllocator::create_oneshot(device_ptr_);

  /* Prepare sine offset data */
  m_sine_offset_data_buffer_size = 0;
  for (uint32_t n_sine_pair = 0; n_sine_pair < N_SINE_PAIRS + 1; ++n_sine_pair) {
	  if (n_sine_pair < N_SINE_PAIRS) {
		  /* Store current data offset */
		  anvil_assert((m_sine_offset_data_buffer_size 
			  % sb_data_alignment_requirement) == 0);
		  m_sine_offset_data_buffer_offsets.push_back(
			  m_sine_offset_data_buffer_size);
	  }

	  /* Account for space necessary to hold a vec2 and any padding required to meet the alignment requirement */
	  m_sine_offset_data_buffer_size += sizeof(float) * 2;
	  m_sine_offset_data_buffer_size += (sb_data_alignment_requirement 
		  - m_sine_offset_data_buffer_size % sb_data_alignment_requirement) 
		  % sb_data_alignment_requirement;
  }

  sine_offset_data_raw_buffer_ptr.reset(
	  new char[static_cast<uintptr_t>(m_sine_offset_data_buffer_size)]);
  for (uint32_t n_sine_pair = 0; n_sine_pair < N_SINE_PAIRS; ++n_sine_pair) {
	  float* sine_pair_data_ptr = (float*)(sine_offset_data_raw_buffer_ptr.get() 
		  + m_sine_offset_data_buffer_offsets[n_sine_pair]);

	  /* Compute the sine start offsets */
	  *sine_pair_data_ptr = -float(2 * (n_sine_pair + 1));
	  sine_pair_data_ptr++;
	  *sine_pair_data_ptr = -float(2 * (n_sine_pair + 1) + 1);
  }

  /* Prepare a buffer object to hold the sine offset data. Note that we fill it with data
  * after memory allocator actually assigns it a memory block.
  */
  m_sine_offset_data_buffer_ptr = Anvil::Buffer::create_nonsparse(device_ptr_,
	  m_sine_offset_data_buffer_size,
	  Anvil::QUEUE_FAMILY_COMPUTE_BIT | Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
	  VK_SHARING_MODE_CONCURRENT,
	  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  m_sine_offset_data_buffer_ptr->set_name("Sine offset data buffer");
  
  // Add the new buffer of uniform data to the memory allocator.
  allocator_ptr->add_buffer(m_sine_offset_data_buffer_ptr,
	  0); /* in_required_memory_features */

  /*
  // Set up a buffer to hold uniform data
  data_buffer_ptr_ = Anvil::Buffer::create_nonsparse(
      device_ptr_, ub_data_size_total,
      Anvil::QUEUE_FAMILY_COMPUTE_BIT | Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
      VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  data_buffer_ptr_->set_name("Data buffer");

  // Add the new buffer of uniform data to the memory allocator.
  allocator_ptr->add_buffer(data_buffer_ptr_, 0);

  // Set up a buffer to hold mesh data 
  mesh_data_buffer_ptr_ = Anvil::Buffer::create_nonsparse(
      device_ptr_, sizeof(g_mesh_data), Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
      VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  mesh_data_buffer_ptr_->set_name("Mesh vertexdata buffer");

  // Add the new buffer of mesh data to the memory allocator.
  allocator_ptr->add_buffer(mesh_data_buffer_ptr_, 0);

  // Allocate memory blocks and copy data where applicable
  mesh_data_buffer_ptr_->write(0, // start_offset 
                               mesh_data_size, mesh_data);
  */

  /* Now prepare a memory block which is going to hold vertex data generated by
  * the producer CS */
  m_sine_data_buffer_size = 0;
  for (unsigned int n_sine_pair = 0; n_sine_pair < N_SINE_PAIRS; ++n_sine_pair) {
	  for (unsigned int n_sine = 0; n_sine < 2; /* sines in a pair */
		  ++n_sine) {

		  /* Store current offset */
		  m_sine_data_buffer_offsets.push_back(m_sine_data_buffer_size);

		  /* Account for space necessary to hold the sine data */
		  m_sine_data_buffer_size += sizeof(float) * 4 
			  /* components */ * N_VERTICES_PER_SINE;

		  /* Pad up as necessary */
		  m_sine_data_buffer_size += (sb_data_alignment_requirement 
			  - m_sine_data_buffer_size % sb_data_alignment_requirement) 
			  % sb_data_alignment_requirement;
		  anvil_assert(m_sine_data_buffer_size 
			  % sb_data_alignment_requirement == 0);
	  }
  }

  m_sine_data_buffer_size *= 2;
  m_sine_data_buffer_ptr = Anvil::Buffer::create_nonsparse(device_ptr_,
	  m_sine_data_buffer_size,
	  Anvil::QUEUE_FAMILY_COMPUTE_BIT | Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
	  VK_SHARING_MODE_CONCURRENT,
	  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
  m_sine_data_buffer_ptr->set_name("Sine data buffer");
  allocator_ptr->add_buffer(m_sine_data_buffer_ptr,
	  0); /* in_required_memory_features */

  /* We also need some space for a uniform block which is going to hold time info. */
  const auto dynamic_ub_alignment_requirement = device_ptr_.lock()
	  ->get_physical_device_properties().limits.minUniformBufferOffsetAlignment;
  const auto sine_props_data_buffer_size_per_swapchain_image = 
	  Anvil::Utils::round_up(sizeof(float), dynamic_ub_alignment_requirement);
  const auto sine_props_data_buffer_size_total = 
	  sine_props_data_buffer_size_per_swapchain_image * N_SWAPCHAIN_IMAGES;
  m_sine_props_data_buffer_size_per_swapchain_image = 
	  sine_props_data_buffer_size_per_swapchain_image;

  m_sine_props_data_buffer_ptr = Anvil::Buffer::create_nonsparse(device_ptr_,
	  sine_props_data_buffer_size_total,
	  Anvil::QUEUE_FAMILY_COMPUTE_BIT | Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
	  VK_SHARING_MODE_CONCURRENT,
	  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  m_sine_props_data_buffer_ptr->set_name("Sine properties data buffer");
  allocator_ptr->add_buffer(m_sine_props_data_buffer_ptr,
	  Anvil::MEMORY_FEATURE_FLAG_MAPPABLE);

  /* Each sine needs to be assigned a different color. 
     Compute the data and upload it to another buffer object. */
  std::unique_ptr<unsigned char> color_buffer_data_ptr;
  unsigned char* color_buffer_data_traveller_ptr = nullptr;
  m_sine_color_buffer_size = N_SINE_PAIRS * 2 /* sines per pair */ 
	  * (2 * sizeof(char)) /* R8G8 */;
  color_buffer_data_ptr.reset(
	  new unsigned char[static_cast<uintptr_t>(m_sine_color_buffer_size)]);
  color_buffer_data_traveller_ptr = color_buffer_data_ptr.get();
  for (uint32_t n_sine = 0; n_sine < 2 * N_SINE_PAIRS; ++n_sine) {
	  *color_buffer_data_traveller_ptr = 
		  (unsigned char)((cos(float(n_sine)) * 0.5f + 0.5f) * 255.0f);
	  color_buffer_data_traveller_ptr++;
	  *color_buffer_data_traveller_ptr = 
		  (unsigned char)((sin(float(n_sine)) * 0.5f + 0.5f) * 255.0f);
	  color_buffer_data_traveller_ptr++;
  }

  m_sine_color_buffer_ptr = Anvil::Buffer::create_nonsparse(device_ptr_,
	  m_sine_color_buffer_size,
	  Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
	  VK_SHARING_MODE_EXCLUSIVE,
	  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  m_sine_color_buffer_ptr->set_name("Sine color data buffer");
  allocator_ptr->add_buffer(m_sine_color_buffer_ptr,
	  0); /* in_required_memory_features */

  /* Assign memory blocks to buffers and fill them with data */
  m_sine_offset_data_buffer_ptr->write(0, /* start_offset */
	  m_sine_offset_data_buffer_ptr->get_size(),
	  sine_offset_data_raw_buffer_ptr.get());
  m_sine_color_buffer_ptr->write(0, /* start_offset */
	  m_sine_color_buffer_ptr->get_size(),
	  color_buffer_data_ptr.get());

  /*
  // COMPUTE BUFFERS.
  const VkDeviceSize comp_per_swap = sizeof(int) * 1;
  const auto comp_alignment_req =
	  device_ptr_.lock()
	  ->get_physical_device_properties()
	  .limits.minUniformBufferOffsetAlignment;
  const auto comp_data_total =
	  N_SWAPCHAIN_IMAGES *
	  (Anvil::Utils::round_up(comp_per_swap,
		  comp_alignment_req));

  printf("sub_c2\n");

  comp_per_swap_ = comp_data_total / N_SWAPCHAIN_IMAGES;

  // Set up a buffer to hold uniform data
  comp_data_buffer_ptr_ = Anvil::Buffer::create_nonsparse(
	  device_ptr_, comp_data_total,
	  Anvil::QUEUE_FAMILY_COMPUTE_BIT | Anvil::QUEUE_FAMILY_GRAPHICS_BIT,
	  VK_SHARING_MODE_EXCLUSIVE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
  comp_data_buffer_ptr_->set_name("Comp data buffer");
  */

  // Add the new buffer of uniform data to the memory allocator.
  // TODO: failing right here.
  // allocator_ptr->add_buffer(comp_data_buffer_ptr_, 0);
}

/*
  DESCRIPTOR SET GROUP INITIALIZATION.
  Creates a descriptor set group, binding uniform data buffers.
 */
void App::init_dsgs() {

	printf("dsg0\n");

	/* Create the descriptor set layouts for the generator program. */
	compute_dsg_ptr_ = Anvil::DescriptorSetGroup::create(device_ptr_,
		false, /* releaseable_sets */
		2      /* n_sets           */);
	printf("dsg1\n");

	compute_dsg_ptr_->add_binding(0, /* n_set      */
		0, /* binding    */
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
		1, /* n_elements */
		VK_SHADER_STAGE_COMPUTE_BIT);
	printf("dsg2\n");

	compute_dsg_ptr_->add_binding(0, /* n_set      */
		1, /* binding    */
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
		1, /* n_elements */
		VK_SHADER_STAGE_COMPUTE_BIT);
	printf("dsg3\n");

	compute_dsg_ptr_->add_binding(1, /* n_set      */
		0, /* binding    */
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		1, /* n_elements */
		VK_SHADER_STAGE_COMPUTE_BIT);
	printf("dsg4\n");

	compute_dsg_ptr_->set_binding_item(0, /* n_set         */
		0, /* binding_index */
		Anvil::DescriptorSet::DynamicStorageBufferBindingElement(m_sine_offset_data_buffer_ptr,
			0, /* in_start_offset */
			sizeof(float) * 2));
	compute_dsg_ptr_->set_binding_item(0, /* n_set         */
		1, /* binding_index */
		Anvil::DescriptorSet::DynamicStorageBufferBindingElement(m_sine_data_buffer_ptr,
			0, /* in_start_offset */
			sizeof(float) * 4 * N_VERTICES_PER_SINE * 2));
	compute_dsg_ptr_->set_binding_item(1, /* n_set         */
		0, /* binding_index */
		Anvil::DescriptorSet::DynamicUniformBufferBindingElement(m_sine_props_data_buffer_ptr,
			0, /* in_start_offset */
			m_sine_props_data_buffer_size_per_swapchain_image));

	/* Set up the descriptor set layout for the renderer program.  */
	dsg_ptr_ = Anvil::DescriptorSetGroup::create(device_ptr_,
		false, /* releaseable_sets */
		2      /* n_sets           */);


	dsg_ptr_->add_binding(0, /* n_set      */
		0, /* binding    */
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
		1, /* n_elements */
		VK_SHADER_STAGE_VERTEX_BIT);
	dsg_ptr_->add_binding(1, /* n_set      */
		0, /* binding    */
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
		1, /* n_elements */
		VK_SHADER_STAGE_VERTEX_BIT);

	dsg_ptr_->set_binding_item(0, /* n_set         */
		0, /* binding_index */
		Anvil::DescriptorSet::DynamicStorageBufferBindingElement(m_sine_data_buffer_ptr,
			0, /* in_start_offset */
			sizeof(float) * 4 * N_VERTICES_PER_SINE));
	dsg_ptr_->set_binding_item(1, /* n_set         */
		0, /* binding_index */
		Anvil::DescriptorSet::DynamicStorageBufferBindingElement(m_sine_data_buffer_ptr,
			0, /* in_start_offset */
			sizeof(float) * 4 * N_VERTICES_PER_SINE));
}

/*
  FRAME BUFFER INITIALIZATION.
  This function creates, for every image in the swapchain, a series of
  most-optimal
  tasks at each step.
 */
void App::init_framebuffers() {
  // Instantiate a framebuffer object for each swapchain image.
  for (uint32_t n_fbo = 0; n_fbo < N_SWAPCHAIN_IMAGES; ++n_fbo) {
    // Retrieve a pointer to the image.
    std::shared_ptr<Anvil::ImageView> attachment_image_view_ptr;
    attachment_image_view_ptr = swapchain_ptr_->get_image_view(n_fbo);

    // Create a framebuffer entry for this one-layer image.
    fbos_[n_fbo] =
        Anvil::Framebuffer::create(device_ptr_, WINDOW_WIDTH, WINDOW_HEIGHT, 1);
    fbos_[n_fbo]->set_name_formatted("Framebuffer for swapchain image [%d]",
                                      n_fbo);

    // Attach this view to the growing list.
    bool result = fbos_[n_fbo]->add_attachment(attachment_image_view_ptr,
		nullptr); /* out_opt_attachment_id_ptr */
	anvil_assert(result);

	result = fbos_[n_fbo]->add_attachment(m_depth_image_views[n_fbo],
		nullptr); /* out_opt_attachment_id_ptr */
  }
}

/*
  SEMAPHORE INITIALIZATION.
  Initialize the sempahores that we use here to ensure proper order and
  correctness.
 */
void App::init_semaphores() {
  // Iterate through all associated semaphores.
  for (uint32_t n_semaphore = 0; n_semaphore < n_swapchain_images_; ++n_semaphore) {
    // Retrieve pointers to the semaphore grabs.
    std::shared_ptr<Anvil::Semaphore> new_signal_semaphore_ptr =
        Anvil::Semaphore::create(device_ptr_);
    std::shared_ptr<Anvil::Semaphore> new_wait_semaphore_ptr =
        Anvil::Semaphore::create(device_ptr_);

    // Display semaphore update information.
    new_signal_semaphore_ptr->set_name_formatted("Signal semaphore [%d]",
                                                 n_semaphore);
    new_wait_semaphore_ptr->set_name_formatted("Wait semaphore [%d]",
                                               n_semaphore);

    // Push new semaphore data.
    frame_signal_semaphores_.push_back(new_signal_semaphore_ptr);
    frame_wait_semaphores_.push_back(new_wait_semaphore_ptr);
  }
}

// Display the interesting output of the shaders!
void App::init_shaders() {
  std::stringstream buffer;
  std::ifstream t;

  t.open("../src/shaders/example.frag");
  buffer << t.rdbuf();
  t.close();
  std::string frag = buffer.str();
  
  buffer.str(std::string());

  t.open("../src/shaders/example.vert");
  buffer << t.rdbuf();
  t.close();
  std::string vert = buffer.str();

  buffer.str(std::string());
  t.open("shaders/example.comp");
  buffer << t.rdbuf();
  t.close();
  std::string compute = buffer.str();

  std::shared_ptr<Anvil::GLSLShaderToSPIRVGenerator> compute_shader_ptr;
  std::shared_ptr<Anvil::ShaderModule> compute_shader_module_ptr;
  std::shared_ptr<Anvil::GLSLShaderToSPIRVGenerator> fragment_shader_ptr;
  std::shared_ptr<Anvil::ShaderModule> fragment_shader_module_ptr;
  std::shared_ptr<Anvil::GLSLShaderToSPIRVGenerator> vertex_shader_ptr;
  std::shared_ptr<Anvil::ShaderModule> vertex_shader_module_ptr;

  compute_shader_ptr = Anvil::GLSLShaderToSPIRVGenerator::create(
	  device_ptr_, Anvil::GLSLShaderToSPIRVGenerator::MODE_USE_SPECIFIED_SOURCE,
	  compute.c_str(), Anvil::SHADER_STAGE_COMPUTE);
  fragment_shader_ptr = Anvil::GLSLShaderToSPIRVGenerator::create(
      device_ptr_, Anvil::GLSLShaderToSPIRVGenerator::MODE_USE_SPECIFIED_SOURCE,
      frag.c_str(), Anvil::SHADER_STAGE_FRAGMENT);
  vertex_shader_ptr = Anvil::GLSLShaderToSPIRVGenerator::create(
      device_ptr_, Anvil::GLSLShaderToSPIRVGenerator::MODE_USE_SPECIFIED_SOURCE,
      vert.c_str(), Anvil::SHADER_STAGE_VERTEX);

  // compute_shader_ptr->add_definition_value_pair("N_TRIANGLES", N_TRIANGLES);
  // fragment_shader_ptr->add_definition_value_pair("N_TRIANGLES", N_TRIANGLES);
  // vertex_shader_ptr->add_definition_value_pair("N_TRIANGLES", N_TRIANGLES);

  /* Set up GLSLShader instances */
  compute_shader_ptr->add_definition_value_pair("N_SINE_PAIRS",
	  N_SINE_PAIRS);
  compute_shader_ptr->add_definition_value_pair("N_VERTICES_PER_SINE",
	  N_VERTICES_PER_SINE);

  vertex_shader_ptr->add_definition_value_pair("N_VERTICES_PER_SINE",
	  N_VERTICES_PER_SINE);

  compute_shader_module_ptr = Anvil::ShaderModule::create_from_spirv_generator(
	  device_ptr_, compute_shader_ptr);
  fragment_shader_module_ptr = Anvil::ShaderModule::create_from_spirv_generator(
      device_ptr_, fragment_shader_ptr);
  vertex_shader_module_ptr = Anvil::ShaderModule::create_from_spirv_generator(
      device_ptr_, vertex_shader_ptr);

  compute_shader_module_ptr->set_name("Compute shader module");
  fragment_shader_module_ptr->set_name("Fragment shader module");
  vertex_shader_module_ptr->set_name("Vertex shader module");

  cs_ptr_.reset(new Anvil::ShaderModuleStageEntryPoint(
	  "main", compute_shader_module_ptr, Anvil::SHADER_STAGE_COMPUTE));
  fs_ptr_.reset(new Anvil::ShaderModuleStageEntryPoint(
      "main", fragment_shader_module_ptr, Anvil::SHADER_STAGE_FRAGMENT));
  vs_ptr_.reset(new Anvil::ShaderModuleStageEntryPoint(
      "main", vertex_shader_module_ptr, Anvil::SHADER_STAGE_VERTEX));
}

/*
  COMPUTE PIPELINE INITIALIZATION.
  Link and setup the several stages of this application with compute steps.
 */
void App::init_compute_pipelines() {
	std::shared_ptr<Anvil::SGPUDevice> device_locked_ptr(device_ptr_);
	std::shared_ptr<Anvil::ComputePipelineManager> compute_manager_ptr(
		device_locked_ptr->get_compute_pipeline_manager());
	bool result;

	/* Create & configure the compute pipeline */
	result = compute_manager_ptr->add_regular_pipeline(
		false, /* disable_optimizations */
		false, /* allow_derivatives     */
		*cs_ptr_,
		&compute_pipeline_id_);
	anvil_assert(result);

	result = compute_manager_ptr->attach_push_constant_range_to_pipeline(
		compute_pipeline_id_,
		0,  /* offset */
		4,  /* size   */
		VK_SHADER_STAGE_COMPUTE_BIT);
	anvil_assert(result);

	result = compute_manager_ptr->set_pipeline_dsg(compute_pipeline_id_,
		compute_dsg_ptr_);
	anvil_assert(result);

	result = compute_manager_ptr->bake();
	anvil_assert(result);
}


/*
  GRAPHICS PIPELINE INITIALIZATION.
  Link together important steps needed for rendering in phases--the pipeline
  steps.
 */
void App::init_gfx_pipelines() {
  std::shared_ptr<Anvil::SGPUDevice> device_locked_ptr(device_ptr_);
  std::shared_ptr<Anvil::GraphicsPipelineManager> gfx_pipeline_manager_ptr(
      device_locked_ptr->get_graphics_pipeline_manager());
  bool result;

  // Create a renderpass for the pipeline.
  Anvil::RenderPassAttachmentID render_pass_color_attachment_id;
  Anvil::RenderPassAttachmentID render_pass_depth_attachment_id;
  Anvil::SubPassID render_pass_subpass_id;
  renderpass_ptr_ = Anvil::RenderPass::create(device_ptr_, swapchain_ptr_);
  renderpass_ptr_->set_name("Main renderpass");

  // Attach to this renderpass.
  result = renderpass_ptr_->add_color_attachment(
      swapchain_ptr_->get_image_format(), VK_SAMPLE_COUNT_1_BIT,
      VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
      VK_IMAGE_LAYOUT_UNDEFINED,
#ifdef ENABLE_OFFSCREEN_RENDERING
      VK_IMAGE_LAYOUT_GENERAL,
#else
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
#endif
      false, /* may_alias */
      &render_pass_color_attachment_id);
  anvil_assert(result);

  // Add a depth stencil.
  result = renderpass_ptr_->add_depth_stencil_attachment(
	  m_depth_images[0]->get_image_format(),
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

  // Add a subpass to the renderpass.
  result = renderpass_ptr_->add_subpass(
      *fs_ptr_, 
	  Anvil::ShaderModuleStageEntryPoint(), /* geometry_shader */
      Anvil::ShaderModuleStageEntryPoint(), /* tess_control_shader    */
      Anvil::ShaderModuleStageEntryPoint(), /* tess_evaluation_shader */
      *vs_ptr_, 
	  &render_pass_subpass_id);
  anvil_assert(result);

  result = renderpass_ptr_->get_subpass_graphics_pipeline_id(
      render_pass_subpass_id, &pipeline_id_);
  anvil_assert(result);

  result = renderpass_ptr_->add_subpass_color_attachment(
      render_pass_subpass_id, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      render_pass_color_attachment_id, 0, /* location                      */
      nullptr);                           /* opt_attachment_resolve_id_ptr */
  result &= renderpass_ptr_->add_subpass_depth_stencil_attachment(
	  render_pass_subpass_id,
	  render_pass_depth_attachment_id,
	  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  anvil_assert(result);

  /* Set up the graphics pipeline for the main subpass */
  /*
  result = gfx_pipeline_manager_ptr->set_pipeline_dsg(pipeline_id_, dsg_ptr_);
  result &= gfx_pipeline_manager_ptr->attach_push_constant_range_to_pipeline(
      pipeline_id_, 0, // offset
      sizeof(float) * 4 * 4, // vec4 values 
      VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT);
  anvil_assert(result);

  gfx_pipeline_manager_ptr->set_rasterization_properties(
      pipeline_id_, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE,
      VK_FRONT_FACE_COUNTER_CLOCKWISE, 1.0f); // line_width
  gfx_pipeline_manager_ptr->set_color_blend_attachment_properties(
      pipeline_id_, 0, // attachment_id
      true,             // blending_enabled
      VK_BLEND_OP_ADD, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA,
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_FACTOR_SRC_ALPHA,
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT |
          VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT);

  result = gfx_pipeline_manager_ptr->add_vertex_attribute(
      pipeline_id_, 0, // location
      VK_FORMAT_R32G32B32A32_SFLOAT, g_mesh_data_position_start_offset,
      g_mesh_data_position_stride, VK_VERTEX_INPUT_RATE_VERTEX);
  anvil_assert(result);

  result = gfx_pipeline_manager_ptr->add_vertex_attribute(
      pipeline_id_, 1, // location
      VK_FORMAT_R32G32B32A32_SFLOAT, g_mesh_data_position_start_offset,
      g_mesh_data_position_stride, VK_VERTEX_INPUT_RATE_VERTEX);
  anvil_assert(result);
  */

  result = renderpass_ptr_->get_subpass_graphics_pipeline_id(
	  render_pass_subpass_id,
	  &pipeline_id_);
  anvil_assert(result);

  gfx_pipeline_manager_ptr->add_vertex_attribute(pipeline_id_,
	  0, /* location */
	  VK_FORMAT_R8G8_UNORM,
	  0,                /* offset_in_bytes */
	  sizeof(char) * 2, /* stride_in_bytes */
	  VK_VERTEX_INPUT_RATE_INSTANCE);
  gfx_pipeline_manager_ptr->set_pipeline_dsg(pipeline_id_,
	  dsg_ptr_);
  gfx_pipeline_manager_ptr->set_input_assembly_properties(pipeline_id_,
	  VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
  gfx_pipeline_manager_ptr->set_rasterization_properties(pipeline_id_,
	  VK_POLYGON_MODE_FILL,
	  VK_CULL_MODE_NONE,
	  VK_FRONT_FACE_COUNTER_CLOCKWISE,
	  1.0f /* line_width */);
  gfx_pipeline_manager_ptr->toggle_depth_test(pipeline_id_,
	  true, /* should_enable */
	  VK_COMPARE_OP_LESS_OR_EQUAL);
  gfx_pipeline_manager_ptr->toggle_depth_writes(pipeline_id_,
	  true); /* should_enable */
  gfx_pipeline_manager_ptr->toggle_dynamic_states(pipeline_id_,
	  true, /* should_enable */
	  Anvil::GraphicsPipelineManager::DYNAMIC_STATE_LINE_WIDTH_BIT);
}

/*
   IMAGE INITIALIZATION.
   Drawn from the AMD DynamicBuffer example.
 */
void App::init_images() {
	for (uint32_t n_depth_image = 0;
	n_depth_image < N_SWAPCHAIN_IMAGES;
		++n_depth_image) {
		m_depth_images[n_depth_image] = Anvil::Image::create_nonsparse(device_ptr_,
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

		m_depth_image_views[n_depth_image] = Anvil::ImageView::create_2D(device_ptr_,
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

/*
  COMMAND BUFFER INITIALIZATION.
  Special thanks to the provided AMD examples for most of this code again.
 */

// Helper function to get the luminance data.
void App::get_luminance_data(std::shared_ptr<float>* out_result_ptr,
                             uint32_t* out_result_size_ptr) const {
  std::shared_ptr<float> luminance_data_ptr;
  float* luminance_data_raw_ptr;
  uint32_t luminance_data_size;

  static_assert(
      N_TRIANGLES == 16,
      "Shader and the app logic assumes N_TRIANGLES will always be 16");

  luminance_data_size = sizeof(float) * N_TRIANGLES;

  luminance_data_ptr.reset(new float[luminance_data_size / sizeof(float)],
                           std::default_delete<float[]>());

  luminance_data_raw_ptr = luminance_data_ptr.get();

  for (uint32_t n_tri = 0; n_tri < N_TRIANGLES; ++n_tri) {
    luminance_data_raw_ptr[n_tri] = float(n_tri) / float(N_TRIANGLES - 1);
  }

  *out_result_ptr = luminance_data_ptr;
  *out_result_size_ptr = luminance_data_size;
}

// Actually intialize the command buffers.
void App::init_command_buffers() {
	std::shared_ptr<Anvil::SGPUDevice> device_locked_ptr(device_ptr_);
	std::shared_ptr<Anvil::GraphicsPipelineManager> gfx_pipeline_manager_ptr(
		device_locked_ptr->get_graphics_pipeline_manager());

	// NEW.
	const bool is_debug_marker_ext_present(device_locked_ptr
		->is_ext_debug_marker_extension_enabled());
	std::shared_ptr<Anvil::PipelineLayout> producer_pipeline_layout_ptr;

	VkImageSubresourceRange image_subresource_range;

	// std::shared_ptr<float> luminance_data_ptr;
	// uint32_t luminance_data_size;
	std::shared_ptr<Anvil::Queue> universal_queue_ptr(
		device_locked_ptr->get_universal_queue(0));

	producer_pipeline_layout_ptr = device_locked_ptr->get_compute_pipeline_manager()
		->get_compute_pipeline_layout(compute_pipeline_id_);

	image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_subresource_range.baseArrayLayer = 0;
	image_subresource_range.baseMipLevel = 0;
	image_subresource_range.layerCount = 1;
	image_subresource_range.levelCount = 1;

	// get_luminance_data(&luminance_data_ptr, &luminance_data_size);

	/* Set up rendering command buffers. We need one per swap-chain image. */
	for (uint32_t n_command_buffer = 0; n_command_buffer < N_SWAPCHAIN_IMAGES;
	++n_command_buffer) {
		std::shared_ptr<Anvil::PrimaryCommandBuffer> cmd_buffer_ptr;
		cmd_buffer_ptr =
			device_locked_ptr->get_command_pool(Anvil::QUEUE_FAMILY_TYPE_UNIVERSAL)
			->alloc_primary_level_command_buffer();

		/* Start recording commands */
		cmd_buffer_ptr->start_recording(false, /* one_time_submit          */
			true); /* simultaneous_use_allowed */

/* Switch the swap-chain image to the color_attachment_optimal image layout
 */
		{
			Anvil::ImageBarrier image_barrier(
				0,                                    /* source_access_mask       */
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, /* destination_access_mask  */
				false, VK_IMAGE_LAYOUT_UNDEFINED,     /* old_image_layout */
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, /* new_image_layout */
				universal_queue_ptr->get_queue_family_index(),
				universal_queue_ptr->get_queue_family_index(),
				swapchain_ptr_->get_image(n_command_buffer), image_subresource_range);

			cmd_buffer_ptr->record_pipeline_barrier(
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, /* src_stage_mask */
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, /* dst_stage_mask */
				VK_FALSE, /* in_by_region                   */
				0,        /* in_memory_barrier_count        */
				nullptr,  /* in_memory_barrier_ptrs         */
				0,        /* in_buffer_memory_barrier_count */
				nullptr,  /* in_buffer_memory_barrier_ptrs  */
				1,        /* in_image_memory_barrier_count  */
				&image_barrier);
		}

		/*
		// Make sure CPU-written data is flushed before we start rendering
		Anvil::BufferBarrier buffer_barrier(
			VK_ACCESS_HOST_WRITE_BIT,   // in_source_access_mask
			VK_ACCESS_UNIFORM_READ_BIT, // in_destination_access_mask
			universal_queue_ptr
				->get_queue_family_index(), // in_src_queue_family_index
			universal_queue_ptr
				->get_queue_family_index(), // in_dst_queue_family_index
			data_buffer_ptr_,
			ub_data_size_per_swapchain_image_ * n_command_buffer, // in_offset
			ub_data_size_per_swapchain_image_);
		*/

		/* Invalidate shader read cache. This is needed because t-value
		[sine_props_data] is written by CPU.
		*
		* We do not need to worry about offset buffer contents getting
		overwritten by subsequent frames because we do not render frames
		ahead of time in this example.
		*/
		Anvil::BufferBarrier t_value_buffer_barrier =
			Anvil::BufferBarrier(VK_ACCESS_HOST_WRITE_BIT, /* in_source_access_mask      */
				VK_ACCESS_UNIFORM_READ_BIT, /* in_destination_access_mask */
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				m_sine_props_data_buffer_ptr,
				n_command_buffer * m_sine_props_data_buffer_size_per_swapchain_image,
				sizeof(float));

		cmd_buffer_ptr->record_pipeline_barrier(
			VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			VK_FALSE,           /* in_by_region                   */
			0,                  /* in_memory_barrier_count        */
			nullptr,            /* in_memory_barriers_ptr         */
			1,                  /* in_buffer_memory_barrier_count */
			&t_value_buffer_barrier, 0, /* in_image_memory_barrier_count  */
			nullptr);           /* in_image_memory_barriers_ptr   */

		// 2. Generate sine offset data using compute shader.
		cmd_buffer_ptr->record_bind_pipeline(VK_PIPELINE_BIND_POINT_COMPUTE,
			compute_pipeline_id_);

		if (is_debug_marker_ext_present) {
			static const float region_color[4] = {
				0.0f,
				1.0f,
				0.0f,
				1.0f };
			cmd_buffer_ptr->record_debug_marker_begin_EXT("Sine offset data computation",
				region_color);
		}

		for (unsigned int n_sine_pair = 0; n_sine_pair < N_SINE_PAIRS; ++n_sine_pair) {
			uint32_t dynamic_offsets[3];
			const uint32_t n_dynamic_offsets = sizeof(dynamic_offsets)
				/ sizeof(dynamic_offsets[0]);
			std::shared_ptr<Anvil::DescriptorSet> producer_dses[] = {
				compute_dsg_ptr_->get_descriptor_set(0),
				compute_dsg_ptr_->get_descriptor_set(1) };
			static const uint32_t n_producer_dses = sizeof(producer_dses)
				/ sizeof(producer_dses[0]);

			get_buffer_memory_offsets(n_sine_pair,
				dynamic_offsets + 1,  /* out_opt_sine1SB_offset_ptr */
				nullptr,              /* out_opt_sine2SB_offset_ptr */
				dynamic_offsets + 0); /* out_opt_offset_data_ptr    */
			dynamic_offsets[2] = static_cast<uint32_t>(
				m_sine_props_data_buffer_size_per_swapchain_image
				* n_command_buffer);

			cmd_buffer_ptr->record_bind_descriptor_sets(VK_PIPELINE_BIND_POINT_COMPUTE,
				producer_pipeline_layout_ptr,
				0, /* firstSet */
				n_producer_dses,
				producer_dses,
				n_dynamic_offsets,
				dynamic_offsets);

			cmd_buffer_ptr->record_push_constants(producer_pipeline_layout_ptr,
				VK_SHADER_STAGE_COMPUTE_BIT,
				0, /* in_offset */
				4, /* in_size   */
				&n_sine_pair);

			cmd_buffer_ptr->record_dispatch(2, /* x */
				1, /* y */
				1); /* z */
		}

		if (is_debug_marker_ext_present) {
			cmd_buffer_ptr->record_debug_marker_end_EXT();
		}

		/* Before we proceed with drawing, we need to flush the buffer data.
		This step is needed in order to ensure that the data we have generated
		in CS is actually visible to the draw call. */
		Anvil::BufferBarrier vertex_buffer_barrier(VK_ACCESS_SHADER_WRITE_BIT,
			/* in_source_access_mask */
			VK_ACCESS_SHADER_READ_BIT,  /* in_destination_access_mask */
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			m_sine_data_buffer_ptr,
			0, /* in_offset */
			m_sine_data_buffer_size);

		cmd_buffer_ptr->record_pipeline_barrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			VK_FALSE, /* in_by_region                   */
			0,        /* in_memory_barrier_count        */
			nullptr,  /* in_memory_barriers_ptr         */
			1,        /* in_buffer_memory_barrier_count */
			&vertex_buffer_barrier,
			0,        /* in_image_memory_barrier_count */
			nullptr); /* in_image_memory_barriers_ptr  */

		/* 3. Render the geometry. */
		VkClearValue attachment_clear_value[2];
		VkRect2D render_area;
		// VkShaderStageFlags shaderStageFlags = 0;

		attachment_clear_value[0].color.float32[0] = 0.25f;
		attachment_clear_value[0].color.float32[1] = 0.5f;
		attachment_clear_value[0].color.float32[2] = 0.75f;
		attachment_clear_value[0].color.float32[3] = 1.0f;
		attachment_clear_value[1].depthStencil.depth = 1.0f;

		//attachment_clear_value.color.float32[0] = 1.0f;
		//attachment_clear_value.color.float32[1] = 0.5f;
		//attachment_clear_value.color.float32[2] = 0.2f;
		//attachment_clear_value.color.float32[3] = 1.0f;

		render_area.extent.height = WINDOW_HEIGHT;
		render_area.extent.width = WINDOW_WIDTH;
		render_area.offset.x = 0;
		render_area.offset.y = 0;

		/* NOTE: The render-pass switches the swap-chain image back to the presentable layout
		*  after the draw call finishes.
		*/
		cmd_buffer_ptr->record_begin_render_pass(
			2, /* in_n_clear_values */
			attachment_clear_value, fbos_[n_command_buffer], render_area,
			renderpass_ptr_, VK_SUBPASS_CONTENTS_INLINE);
		{
			/*
			const uint32_t data_ub_offset = static_cast<uint32_t>(
				ub_data_size_per_swapchain_image_ * n_command_buffer);
			std::shared_ptr<Anvil::DescriptorSet> ds_ptr =
				dsg_ptr_->get_descriptor_set(0); // n set
			const VkDeviceSize mesh_data_buffer_offset = 0;
			std::shared_ptr<Anvil::PipelineLayout> pipeline_layout_ptr =
				gfx_pipeline_manager_ptr->get_graphics_pipeline_layout(pipeline_id_);

			cmd_buffer_ptr->record_bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS,
												 pipeline_id_);

			cmd_buffer_ptr->record_push_constants(
				pipeline_layout_ptr,
				VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
				0, // in_offset
				luminance_data_size, luminance_data_ptr.get());

			cmd_buffer_ptr->record_bind_descriptor_sets(
				VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_ptr,
				0,                // firstSet
				1,                // setCount
				&ds_ptr, 1,       // dynamicOffsetCount
				&data_ub_offset); // pDynamicOffsets

			cmd_buffer_ptr->record_bind_vertex_buffers(0, // startBinding
													   1, // bindingCount
													   &mesh_data_buffer_ptr_,
													   &mesh_data_buffer_offset);

			cmd_buffer_ptr->record_draw(3,           // in_vertex_count
										N_TRIANGLES, // in_instance_count
										0,           // in_first_vertex
										0);          // in_first_instance
		  }
		  cmd_buffer_ptr->record_end_render_pass();

		  // Close the recording process
		  cmd_buffer_ptr->stop_recording();

		  command_buffers_[n_command_buffer] = cmd_buffer_ptr;
		  */

			const float max_line_width = device_ptr_.lock()
				->get_physical_device_properties().limits.lineWidthRange[1];
			std::shared_ptr<Anvil::DescriptorSet> renderer_dses[] = {
				compute_dsg_ptr_->get_descriptor_set(0),
				compute_dsg_ptr_->get_descriptor_set(1) };
			const uint32_t n_renderer_dses = sizeof(renderer_dses)
				/ sizeof(renderer_dses[0]);
			std::shared_ptr<Anvil::PipelineLayout> renderer_pipeline_layout_ptr;
			static const VkDeviceSize              sine_color_buffer_start_offset = 0;
			renderer_pipeline_layout_ptr =
				gfx_pipeline_manager_ptr->get_graphics_pipeline_layout(
					compute_pipeline_id_);

			cmd_buffer_ptr->record_bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS,
				compute_pipeline_id_);
			cmd_buffer_ptr->record_bind_vertex_buffers(0, /* startBinding */
				1, /* bindingCount */
				&m_sine_color_buffer_ptr,
				&sine_color_buffer_start_offset);

			for (unsigned int n_sine_pair = 0; n_sine_pair < N_SINE_PAIRS; ++n_sine_pair) {
				uint32_t dynamic_offsets[2];
				float new_line_width = float(n_sine_pair + 1) * 3;
				const uint32_t n_dynamic_offsets = sizeof(dynamic_offsets)
					/ sizeof(dynamic_offsets[0]);

				/* Clamp the line width */
				if (new_line_width > max_line_width) {
					new_line_width = max_line_width;
				}

				if (is_debug_marker_ext_present) {
					std::stringstream marker_name_sstream;
					marker_name_sstream << "Draw sine pair "
						<< n_sine_pair;
					cmd_buffer_ptr->record_debug_marker_insert_EXT(
						marker_name_sstream.str().c_str(),
						nullptr);
				}

				get_buffer_memory_offsets(n_sine_pair,
					dynamic_offsets + 0,  /* out_opt_sine1SB_offset_ptr */
					dynamic_offsets + 1); /* out_opt_sine2SB_offset_ptr */
				cmd_buffer_ptr->record_set_line_width(new_line_width);

				cmd_buffer_ptr->record_bind_descriptor_sets(VK_PIPELINE_BIND_POINT_GRAPHICS,
					renderer_pipeline_layout_ptr,
					0, /* firstSet */
					n_renderer_dses,
					renderer_dses,
					n_dynamic_offsets,
					dynamic_offsets);

				cmd_buffer_ptr->record_draw(N_VERTICES_PER_SINE,
					2,                /* instanceCount */
					0,                /* firstVertex   */
					n_sine_pair * 2); /* firstInstance */
			}
		}

		cmd_buffer_ptr->record_end_render_pass();

		/* Close the recording process */
		cmd_buffer_ptr->stop_recording();

		command_buffers_[n_command_buffer] = cmd_buffer_ptr;
	}
}

void App::init_camera() {
  camera_  = Camera();
  camera_.UpdateView();
  camera_.UpdateProj();
  camera_.GetViewProj().Print();
  //window_ptr_->register_for_callbacks(
  //    Anvil::WINDOW_CALLBACK_ID_KEYPRESS_RELEASED, on_keypress_event, this);
  //auto fun = std::bind(&App::on_keypress_event, this, std::placeholders::_1,
  //                     std::placeholders::_2, std::placeholders::_3,
  //                     std::placeholders::_4, std::placeholders::_5);
  //glfwSetKeyCallback(GetGLFWWindow(), &fun);
  //glfwSetKeyCallback(
  //    GetGLFWWindow(),
  //    std::bind(&App::on_keypress_event, this, std::placeholders::_1,
  //              std::placeholders::_2, std::placeholders::_3,
  //              std::placeholders::_4, std::placeholders::_5));
  Callback::GetInstance()->init(this, &camera_);
  glfwSetKeyCallback(GetGLFWWindow(), Callback::on_keypress_event);
  glfwSetMouseButtonCallback(GetGLFWWindow(), Callback::on_mouse_button_event);
  glfwSetCursorPosCallback(GetGLFWWindow(), Callback::on_mouse_move_event);
  glfwSetScrollCallback(GetGLFWWindow(), Callback::on_mouse_scroll_event);

  glfwSetInputMode(GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void App::handle_keys() {
  auto keys = Callback::GetInstance()->get_keys();
  //std::cout << "keys: ";
  for (int key : *keys) {
    //std::cout << (char)key << " ";
    switch (key) {
      case 'w': case 'W':
        camera_.MoveForward(0.1);
        break;
      case 's': case 'S':
        camera_.MoveBackward(0.1);
        break;
      case 'a': case 'A':
        camera_.MoveLeft(0.1);
        break;
      case 'd': case 'D':
        camera_.MoveRight(0.1);
        break;
      case 'q': case 'Q':
        camera_.MoveAna(0.1);
        break;
      case 'e': case 'E':
        camera_.MoveKata(0.1);
        break;
      case 'r': case 'R':
        camera_.MoveUp(0.1);
        break;
      case 'f': case 'F':
        camera_.MoveDown(0.1);
        break;
    }
  }
  //std::cout << "\n";
  camera_.GetViewProj().Print();
}

/*
  The main development portion of the code, now that boilerplate
  and pipeline setup is completed.
 */

/*
  Updates the buffer memory, which holds position, rotation and size data for
  all triangles.
 */
void App::update_data_ub_contents(uint32_t in_n_swapchain_image) {
  struct {
    int frame_index[4];              /* frame index + padding (ivec3) */
    float position_rotation[16 * 4]; /* pos (vec2) + rot (vec2)       */
    float size[16];
  } data;

  char* mapped_data_ptr = nullptr;
  static uint32_t n_frames_rendered = 0;
  const float scale_factor = 1.35f;

  data.frame_index[0] = 0;
  // data.frame_index[0] = static_cast<int>(m_time / 2);

  for (unsigned int n_triangle = 0; n_triangle < N_TRIANGLES; ++n_triangle) {
    float x = cos(3.14152965f * 2.0f * float(n_triangle) / float(N_TRIANGLES)) *
              0.5f * scale_factor;
    float y = sin(3.14152965f * 2.0f * float(n_triangle) / float(N_TRIANGLES)) *
              0.5f * scale_factor;

    data.position_rotation[n_triangle * 4 + 0] = x;
    data.position_rotation[n_triangle * 4 + 1] = y;
    data.position_rotation[n_triangle * 4 + 2] =
        float(data.frame_index[0]) / 360.0f +
        3.14152965f * 2.0f * float(n_triangle) / float(N_TRIANGLES);
    data.position_rotation[n_triangle * 4 + 3] =
        float(data.frame_index[0]) / 360.0f +
        3.14152965f * 2.0f * float(n_triangle) / float(N_TRIANGLES);
    data.size[n_triangle] = 0.2f;
  }

  data_buffer_ptr_->write(
      in_n_swapchain_image *
          ub_data_size_per_swapchain_image_, /* start_offset */
      sizeof(data),
      &data, device_ptr_.lock()->get_universal_queue(0));

  ++n_frames_rendered;
}

/*
 Handle the task of drawing a frame for the app.
*/
void App::draw_frame(void* app_raw_ptr) {
  App* app_ptr = static_cast<App*>(app_raw_ptr);
  std::shared_ptr<Anvil::Semaphore> curr_frame_signal_semaphore_ptr;
  std::shared_ptr<Anvil::Semaphore> curr_frame_wait_semaphore_ptr;

  // TODO: This line might need to change.
  std::shared_ptr<Anvil::SGPUDevice> device_locked_ptr =
      app_ptr->device_ptr_.lock();

  static uint32_t n_frames_rendered = 0;
  uint32_t n_swapchain_image;
  std::shared_ptr<Anvil::Semaphore> present_wait_semaphore_ptr;
  const VkPipelineStageFlags wait_stage_mask =
      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  /* Determine the signal + wait semaphores to use for drawing this frame */
  app_ptr->n_last_semaphore_used_ =
      (app_ptr->n_last_semaphore_used_ + 1) % app_ptr->n_swapchain_images_;

  curr_frame_signal_semaphore_ptr =
      app_ptr->frame_signal_semaphores_[app_ptr->n_last_semaphore_used_];
  curr_frame_wait_semaphore_ptr =
      app_ptr->frame_wait_semaphores_[app_ptr->n_last_semaphore_used_];

  present_wait_semaphore_ptr = curr_frame_signal_semaphore_ptr;

  /* Determine the semaphore which the swapchain image */
  n_swapchain_image = app_ptr->swapchain_ptr_->acquire_image(
      curr_frame_wait_semaphore_ptr, true); /* in_should_block */

  // NEW.
  // TODO: Might be wrong.
  /* Update time value, used by the generator compute shader */
  const uint64_t time_msec = app_ptr->m_time.get_time_in_msec();
  const float    t = time_msec / 1000.0f;
  app_ptr->m_sine_props_data_buffer_ptr->write(
	  /* start_offset */
	  app_ptr->m_sine_props_data_buffer_size_per_swapchain_image * n_swapchain_image,
	  sizeof(float), &t);

  /* Submit work chunk and present */
  app_ptr->update_data_ub_contents(n_swapchain_image);

  // Submit job to queues and synchronize.
  std::shared_ptr<Anvil::Queue> present_queue_ptr =
	  device_locked_ptr->get_universal_queue(0);
  present_queue_ptr->submit_command_buffer_with_signal_wait_semaphores(
      app_ptr->command_buffers_[n_swapchain_image],
      1,                                   /* n_semaphores_to_signal */
      &curr_frame_signal_semaphore_ptr, 1, /* n_semaphores_to_wait_on */
      &curr_frame_wait_semaphore_ptr, &wait_stage_mask,
      false /* should_block */);

  present_queue_ptr->present(app_ptr->swapchain_ptr_, n_swapchain_image,
                             1, /* n_wait_semaphores */
                             &present_wait_semaphore_ptr);

  ++n_frames_rendered;
}

void App::run() { //window_ptr_->run(); 
  while(!ShouldQuit()) {
    glfwPollEvents();
    draw_frame(this);
    //auto cur_time = std::chrono::steady_clock::now();
    //std::chrono::duration<double, std::milli> dif = cur_time - prev_time;
    //std::cout << dif.count() << "\n";
    //prev_time = cur_time;
    handle_keys();
  }
  DestroyWindow();
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
