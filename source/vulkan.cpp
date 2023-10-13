#include <algorithm>
#include <buggy/vulkan.hpp>
#include <buggy/window.hpp>
#include <cjdb/contracts.hpp>
#include <expected>
#include <fstream>
#include <iterator>
#include <print>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace vulkan {
	// NOLINTNEXTLINE(misc-use-anonymous-namespace,readability-identifier-naming)
	static void vkDestroyDebugUtilsMessengerEXT(
	  VkInstance const instance,
	  VkDebugUtilsMessengerEXT const messenger,
	  VkAllocationCallbacks const* const allocator) noexcept
	{
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		  vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func != nullptr) {
			func(instance, messenger, allocator);
		}
	}

	std::expected<debug_utils, error> debug_utils::create(
	  VkInstance const instance,
	  severity_t const severity,
	  type_t const type,
	  diagnostic_callback_t const callback,
	  VkAllocationCallbacks const* const allocator) noexcept
	{
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		  vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func == nullptr) {
			return std::unexpected(vulkan::error::extension_unavailable);
		}

		auto const create_info = VkDebugUtilsMessengerCreateInfoEXT{
		  .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		  .pNext = nullptr,
		  .flags = {},
		  .messageSeverity = static_cast<std::uint32_t>(severity),
		  .messageType = static_cast<std::uint32_t>(type),
		  .pfnUserCallback = callback,
		  .pUserData = nullptr,
		};

		VkDebugUtilsMessengerEXT messenger;
		if (func(instance, &create_info, allocator, &messenger) != VK_SUCCESS) {
			return std::unexpected(vulkan::error::no_host_memory);
		}

		return debug_utils(messenger, instance, allocator);
	}

	debug_utils::debug_utils(
	  VkDebugUtilsMessengerEXT const messenger,
	  VkInstance const instance,
	  VkAllocationCallbacks const* const alloc) noexcept
	: messenger_(messenger, {vkDestroyDebugUtilsMessengerEXT, instance, alloc})
	{}

	VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils::log(
	  VkDebugUtilsMessageSeverityFlagBitsEXT,
	  VkDebugUtilsMessageTypeFlagsEXT,
	  VkDebugUtilsMessengerCallbackDataEXT const* data,
	  void*) noexcept
	{
		std::print(stderr, "validation layer: {}\n", data->pMessage);
		return VK_FALSE;
	}

	static bool check_layer_support(std::span<char const* const> const expected_layers) noexcept
	{
		auto to_string_view = [](VkLayerProperties const& properties) { return std::string_view(properties.layerName); };
		CJDB_EXPECTS(std::ranges::is_sorted(expected_layers, {}));

		auto layer_count = std::uint32_t{0};
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		auto available_layers = std::vector<VkLayerProperties>(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
		std::ranges::sort(available_layers, {}, to_string_view);

		auto intersection = std::vector<std::string_view>();
		intersection.reserve(expected_layers.size());
		std::ranges::set_intersection(
		  available_layers | std::views::transform(to_string_view),
		  expected_layers,
		  std::back_inserter(intersection));

		return expected_layers.size() == intersection.size();
	}

	static std::span<char const* const> required_extensions() noexcept
	{
		return window::context::required_extensions();
	}

	std::expected<instance, error> instance::create(
	  VkApplicationInfo const app_info,
	  std::span<char const* const> const layers,
	  std::span<char const* const> extensions) noexcept
	{
		return create(app_info, nullptr, layers, extensions);
	}

	std::expected<instance, error> instance::create(
	  VkApplicationInfo const app_info,
	  VkAllocationCallbacks const* allocator,
	  std::span<char const* const> const layers,
	  std::span<char const* const> extensions) noexcept
	{
		if (not check_layer_support(layers)) {
			return std::unexpected(error::layer_unavailable);
		}

		auto all_extensions = std::vector<char const*>(std::from_range_t{}, required_extensions());
		all_extensions.append_range(extensions);

		auto create_debug_info = VkDebugUtilsMessengerCreateInfoEXT{
		  .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		  .pNext = nullptr,
		  .flags = {},
		  .messageSeverity = static_cast<std::uint32_t>(debug_utils::verbose | debug_utils::warning | debug_utils::error),
		  .messageType = static_cast<std::uint32_t>(debug_utils::general | debug_utils::validation | debug_utils::performance),
		  .pfnUserCallback = debug_utils::log,
		  .pUserData = nullptr,
		};
		auto create_info = VkInstanceCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		  .pNext =
		    std::ranges::find(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != extensions.end() ? &create_debug_info : nullptr,
		  .flags = 0,
		  .pApplicationInfo = &app_info,
		  .enabledLayerCount = static_cast<std::uint32_t>(layers.size()),
		  .ppEnabledLayerNames = layers.data(),
		  .enabledExtensionCount = static_cast<std::uint32_t>(all_extensions.size()),
		  .ppEnabledExtensionNames = all_extensions.data(),
		};

		auto raw_instance = VkInstance();
		if (auto const result = vkCreateInstance(&create_info, allocator, &raw_instance); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return instance(raw_instance, allocator);
	}

	instance::instance(VkInstance const instance, VkAllocationCallbacks const* allocator) noexcept
	: instance_(instance, {vkDestroyInstance, allocator})
	, physical_devices_(retrieve_devices(instance_.get()))
	{}

	std::span<physical_device const> instance::physical_devices() const noexcept
	{
		return physical_devices_;
	}

	[[nodiscard]] static std::string_view extension_to_string(VkExtensionProperties const x) noexcept
	{
		return x.extensionName;
	}

	std::vector<physical_device> instance::retrieve_devices(VkInstance const instance) noexcept
	{
		auto num_devices = std::uint32_t{};
		vkEnumeratePhysicalDevices(instance, &num_devices, nullptr);

		if (num_devices == 0) {
			return {};
		}

		auto devices = std::vector<VkPhysicalDevice>(num_devices);
		vkEnumeratePhysicalDevices(instance, &num_devices, devices.data());

		auto result = std::vector<physical_device>();
		result.reserve(num_devices);
		std::ranges::transform(devices, std::back_inserter(result), [](VkPhysicalDevice const device) noexcept {
			auto properties = VkPhysicalDeviceProperties{};
			vkGetPhysicalDeviceProperties(device, &properties);

			auto features = VkPhysicalDeviceFeatures{};
			vkGetPhysicalDeviceFeatures(device, &features);

			auto memory_properties = VkPhysicalDeviceMemoryProperties{};
			vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

			auto num_extensions = std::uint32_t{0};
			vkEnumerateDeviceExtensionProperties(device, nullptr, &num_extensions, nullptr);

			auto extensions = std::vector<VkExtensionProperties>(num_extensions);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &num_extensions, extensions.data());
			std::ranges::sort(extensions, {}, extension_to_string);

			return physical_device{
			  .device = device,
			  .properties = properties,
			  .features = features,
			  .memory_properties = memory_properties,
			  .extensions = std::move(extensions),
			};
		});

		return result;
	}

	[[nodiscard]] static std::optional<std::uint32_t> find_present_queue(
	  physical_device const& device,
	  VkSurfaceKHR const surface) noexcept
	{
		auto num_families = std::uint32_t{0};
		vkGetPhysicalDeviceQueueFamilyProperties(device.device, &num_families, nullptr);

		auto families = std::vector<VkQueueFamilyProperties>(num_families);
		vkGetPhysicalDeviceQueueFamilyProperties(device.device, &num_families, families.data());

		auto indexed_families = std::views::zip(families, std::views::iota(std::uint32_t{}));
		auto const graphics = std::ranges::find_if(indexed_families, [&](auto const& x) noexcept {
			auto&& [family, i] = x;
			auto result = VkBool32{};
			vkGetPhysicalDeviceSurfaceSupportKHR(device.device, i, surface, &result);
			return static_cast<bool>(family.queueFlags & VK_QUEUE_GRAPHICS_BIT) and result == VK_TRUE;
		});

		if (graphics == indexed_families.end()) {
			return std::nullopt;
		}

		return static_cast<std::uint32_t>(std::get<1>(*graphics));
	}

	[[nodiscard]] static bool supports_extensions(physical_device const& device, std::span<char const* const> const extensions) noexcept
	{
		assert(std::ranges::is_sorted(extensions));
		return std::ranges::includes(device.extensions, extensions, {}, extension_to_string);
	}

	struct swapchain_support_details {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;

		[[nodiscard]] VkSurfaceFormatKHR choose_format() const noexcept
		{
			auto const format = std::ranges::find_if(formats, [](VkSurfaceFormatKHR const format) noexcept {
				return format.format == VK_FORMAT_B8G8R8A8_SRGB and format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			});

			return format != formats.end() ? *format : formats[0];
		}

		[[nodiscard]] VkPresentModeKHR choose_present_mode() const noexcept
		{
			auto const mode = std::ranges::find_if(present_modes, [](VkPresentModeKHR const mode) noexcept {
				return mode == VK_PRESENT_MODE_MAILBOX_KHR;
			});
			return mode != present_modes.end() ? *mode : VK_PRESENT_MODE_FIFO_KHR;
		}

		[[nodiscard]] VkExtent2D choose_extent(GLFWwindow* window) const noexcept
		{
			if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
				return capabilities.currentExtent;
			}

			auto width = 0;
			auto height = 0;
			glfwGetFramebufferSize(window, &width, &height);

			return VkExtent2D{
			  .width =
			    std::clamp(static_cast<std::uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			  .height = std::clamp(
			    static_cast<std::uint32_t>(height),
			    capabilities.minImageExtent.height,
			    capabilities.maxImageExtent.height),
			};
		}

		[[nodiscard]] static swapchain_support_details query(VkPhysicalDevice const device, VkSurfaceKHR const surface) noexcept
		{
			auto result = swapchain_support_details{};
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &result.capabilities);

			auto num_formats = std::uint32_t{0};
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &num_formats, nullptr);
			if (num_formats != 0) {
				result.formats.resize(num_formats);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &num_formats, result.formats.data());
			}

			auto num_present_modes = std::uint32_t{0};
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &num_present_modes, nullptr);
			if (num_present_modes != 0) {
				result.present_modes.resize(num_present_modes);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &num_present_modes, result.present_modes.data());
			}
			return result;
		}
	};

	std::expected<device, error> device::create(
	  instance const& instance,
	  window::window const& window,
	  selector_fn const selector,
	  std::span<char const* const> const extensions,
	  VkAllocationCallbacks const* const allocator) noexcept
	{
		auto physical_devices = instance.physical_devices();
		auto family_index = std::uint32_t{0};
		auto physical_device =
		  std::ranges::find_if(physical_devices, [&selector, &family_index, &window, &extensions](auto const& device) noexcept {
			  if (not (selector(device) and supports_extensions(device, extensions))) {
				  return false;
			  }

			  auto const swapchain_support = swapchain_support_details::query(device.device, window.get_surface());
			  if (swapchain_support.formats.empty() or swapchain_support.present_modes.empty()) {
				  return false;
			  }

			  auto const index = find_present_queue(device, window.get_surface());
			  if (index) {
				  family_index = *index;
			  }

			  return true;
		  });

		if (physical_device == physical_devices.end()) {
			return std::unexpected(error::no_suitable_devices);
		}

		auto queue_priority = 1.0f;
		auto queue_create_info = VkDeviceQueueCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .queueFamilyIndex = family_index,
		  .queueCount = 1,
		  .pQueuePriorities = &queue_priority,
		};

		auto device_create_info = VkDeviceCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .queueCreateInfoCount = 1,
		  .pQueueCreateInfos = &queue_create_info,
		  .enabledLayerCount = 0,
		  .ppEnabledLayerNames = nullptr,
		  .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
		  .ppEnabledExtensionNames = extensions.data(),
		  .pEnabledFeatures = &physical_device->features,
		};

		auto device = VkDevice{};
		auto const result = vkCreateDevice(physical_device->device, &device_create_info, allocator, &device);

		if (result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		auto queue = VkQueue{};
		vkGetDeviceQueue(device, family_index, 0, &queue);

		return vulkan::device(device, queue, allocator, *physical_device);
	}

	error_or<void> device::wait_one(std::span<VkFence const> const fences, std::uint64_t const timeout) noexcept
	{
		auto const result =
		  vkWaitForFences(device_.get(), static_cast<std::uint32_t>(fences.size()), fences.data(), VK_FALSE, timeout);
		if (result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return {};
	}

	error_or<void> device::wait_all(std::span<VkFence const> const fences, std::uint64_t const timeout) noexcept
	{
		auto const result =
		  vkWaitForFences(device_.get(), static_cast<std::uint32_t>(fences.size()), fences.data(), VK_TRUE, timeout);
		if (result != VK_SUCCESS) {
			return std::unexpected(result != VK_TIMEOUT ? static_cast<error>(result) : error::timeout);
		}

		return {};
	}

	error_or<void> device::reset(std::span<VkFence const> fences) noexcept
	{
		auto const result = vkResetFences(device_.get(), static_cast<std::uint32_t>(fences.size()), fences.data());
		if (result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return {};
	}

	error_or<void> device::submit(
	  std::span<VkCommandBuffer> commands,
	  std::span<VkSemaphore> const wait,
	  std::span<VkPipelineStageFlags const> const wait_stages,
	  std::span<VkSemaphore> const signals,
	  fence& f) noexcept
	{
		auto const submit_info = VkSubmitInfo{
		  .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		  .pNext = nullptr,
		  .waitSemaphoreCount = static_cast<std::uint32_t>(wait.size()),
		  .pWaitSemaphores = wait.data(),
		  .pWaitDstStageMask = wait_stages.data(),
		  .commandBufferCount = static_cast<std::uint32_t>(commands.size()),
		  .pCommandBuffers = commands.data(),
		  .signalSemaphoreCount = static_cast<std::uint32_t>(signals.size()),
		  .pSignalSemaphores = signals.data(),
		};

		if (auto const result = vkQueueSubmit(queue_, 1, &submit_info, f.get()); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return {};
	}

	device::device(
	  VkDevice const device,
	  VkQueue const queue,
	  VkAllocationCallbacks const* const allocator,
	  struct physical_device const& physical_device) noexcept
	: device_(device, {vkDestroyDevice, allocator})
	, queue_(queue)
	, physical_device_(&physical_device)
	{}

	std::expected<swapchain, error> swapchain::create(
	  device const& d,
	  window::window const& w,
	  VkAllocationCallbacks const* allocator) noexcept
	{
		auto const support = swapchain_support_details::query(d.physical_device().device, w.get_surface());
		auto const num_images = std::max(support.capabilities.minImageCount + 1, support.capabilities.maxImageCount);
		auto const [image_format, colour_space] = support.choose_format();

		auto create_info = VkSwapchainCreateInfoKHR{
		  .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		  .pNext = nullptr,
		  .flags = {},
		  .surface = w.get_surface(),
		  .minImageCount = num_images,
		  .imageFormat = image_format,
		  .imageColorSpace = colour_space,
		  .imageExtent = support.choose_extent(w.get_window()),
		  .imageArrayLayers = 1,
		  .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // alternatively VK_IMAGE_USAGE_TRANSFER_DST_BIT
		  .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		  .queueFamilyIndexCount = {},
		  .pQueueFamilyIndices = {},
		  .preTransform = support.capabilities.currentTransform,
		  .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		  .presentMode = support.choose_present_mode(),
		  .clipped = VK_TRUE,
		  .oldSwapchain = VK_NULL_HANDLE,
		};

		VkSwapchainKHR resource;
		if (auto const result = vkCreateSwapchainKHR(d.get(), &create_info, allocator, &resource); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return swapchain(resource, d, allocator, image_format, create_info.imageExtent);
	}

	swapchain::swapchain(
	  VkSwapchainKHR const s,
	  device const& d,
	  VkAllocationCallbacks const* allocator,
	  VkFormat format,
	  VkExtent2D extent) noexcept
	: swapchain_(s, {vkDestroySwapchainKHR, d.get(), allocator})
	, format_(format)
	, extent_(extent)
	, device_(d.get())
	{
		auto num_images = std::uint32_t{0};
		vkGetSwapchainImagesKHR(d.get(), s, &num_images, nullptr);
		images_.resize(num_images);
		vkGetSwapchainImagesKHR(d.get(), s, &num_images, images_.data());

		image_views_.reserve(images_.size());
		std::ranges::transform(images_, std::back_inserter(image_views_), [format, allocator, &d](VkImage const image) noexcept {
			return *image_view::create(d, image, format, allocator);
		});
	}

	VkFormat swapchain::format() const noexcept
	{
		return format_;
	}

	VkExtent2D swapchain::extent() const noexcept
	{
		return extent_;
	}

	std::uint32_t swapchain::size() const noexcept
	{
		return static_cast<std::uint32_t>(image_views_.size());
	}

	std::span<image_view const> swapchain::image_views() const
	{
		return image_views_;
	}

	error_or<std::uint32_t> swapchain::acquire_next_image(semaphore& s, std::uint64_t const timeout) noexcept
	{
		auto index = std::uint32_t{};
		if (auto const result = vkAcquireNextImageKHR(device_, swapchain_.get(), timeout, s.get(), VK_NULL_HANDLE, &index);
		    result != VK_SUCCESS)
		{
			return std::unexpected(static_cast<error>(result));
		}

		return index;
	}

	template<VkShaderStageFlagBits kind>
	std::expected<shader_module<kind>, error> shader_module<kind>::create(
	  std::string_view const path,
	  device const& d,
	  VkAllocationCallbacks const* const allocator) noexcept
	{
		auto const bytecode = [path]() noexcept -> std::expected<std::vector<char>, error> {
			auto file = std::ifstream(path);
			if (not file) {
				return std::unexpected(error::file_not_found);
			}
			file.unsetf(std::ios_base::skipws);
			return std::views::istream<char>(file) | std::ranges::to<std::vector>();
		}();

		if (not bytecode) {
			return std::unexpected(bytecode.error());
		}

		auto create_info = VkShaderModuleCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .codeSize = bytecode->size(),
		  .pCode = std::launder(reinterpret_cast<std::uint32_t const*>(bytecode->data())),
		};

		auto module = VkShaderModule{};
		if (auto const result = vkCreateShaderModule(d.get(), &create_info, allocator, &module); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return shader_module(module, d.get(), allocator);
	}

	template<VkShaderStageFlagBits kind>
	VkPipelineShaderStageCreateInfo shader_module<kind>::pipeline_create_info(std::string_view const entry_point_name) const noexcept
	{
		return VkPipelineShaderStageCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .stage = static_cast<VkShaderStageFlagBits>(kind),
		  .module = module_.get(),
		  .pName = entry_point_name.data(),
		  .pSpecializationInfo = {},
		};
	}

	template<VkShaderStageFlagBits kind>
	shader_module<kind>::shader_module(
	  VkShaderModule const module,
	  VkDevice const device,
	  VkAllocationCallbacks const* const allocator) noexcept
	: module_(module, {vkDestroyShaderModule, device, allocator})
	{}

	template class shader_module<VK_SHADER_STAGE_VERTEX_BIT>;
	template class shader_module<VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT>;
	template class shader_module<VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT>;
	template class shader_module<VK_SHADER_STAGE_GEOMETRY_BIT>;
	template class shader_module<VK_SHADER_STAGE_FRAGMENT_BIT>;
	template class shader_module<VK_SHADER_STAGE_COMPUTE_BIT>;

	std::expected<image_view, error> image_view::create(
	  device const& d,
	  VkImage const image,
	  VkFormat const format,
	  VkAllocationCallbacks const* const allocator) noexcept
	{
		auto const subresource_range = VkImageSubresourceRange{
		  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		  .baseMipLevel = 0,
		  .levelCount = 1,
		  .baseArrayLayer = 0,
		  .layerCount = 1,
		};
		auto const components = VkComponentMapping{
		  .r = VK_COMPONENT_SWIZZLE_IDENTITY,
		  .g = VK_COMPONENT_SWIZZLE_IDENTITY,
		  .b = VK_COMPONENT_SWIZZLE_IDENTITY,
		  .a = VK_COMPONENT_SWIZZLE_IDENTITY,
		};
		auto const create_info = VkImageViewCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .image = image,
		  .viewType = VK_IMAGE_VIEW_TYPE_2D,
		  .format = format,
		  .components = components,
		  .subresourceRange = subresource_range,
		};

		auto resource = VkImageView{};
		if (auto const result = vkCreateImageView(d.get(), &create_info, allocator, &resource); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return image_view(resource, d.get(), allocator);
	}

	VkImageView image_view::get() const noexcept
	{
		return view_.get();
	}

	image_view::image_view(VkImageView const i, VkDevice const d, VkAllocationCallbacks const* const allocator) noexcept
	: view_(i, {vkDestroyImageView, d, allocator})
	{}

	error_or<render_pass> render_pass::create(device const& d, swapchain const& s, VkAllocationCallbacks const* const allocator) noexcept
	{
		auto const colour_attachment = VkAttachmentDescription{
		  .flags = {},
		  .format = s.format(),
		  .samples = VK_SAMPLE_COUNT_1_BIT,
		  .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		  .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		  .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		  .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		  .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		  .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};
		constexpr auto colour_attachment_ref = VkAttachmentReference{
		  .attachment = 0,
		  .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		auto const subpass = VkSubpassDescription{
		  .flags = {},
		  .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		  .inputAttachmentCount = 0,
		  .pInputAttachments = nullptr,
		  .colorAttachmentCount = 1,
		  .pColorAttachments = &colour_attachment_ref,
		  .pResolveAttachments = nullptr,
		  .pDepthStencilAttachment = nullptr,
		  .preserveAttachmentCount = 0,
		  .pPreserveAttachments = nullptr,
		};
		constexpr auto subpass_dependency = VkSubpassDependency{
		  .srcSubpass = VK_SUBPASS_EXTERNAL,
		  .dstSubpass = 0,
		  .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		  .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		  .srcAccessMask = 0,
		  .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		  .dependencyFlags = {},
		};
		auto const render_pass_info = VkRenderPassCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .attachmentCount = 1,
		  .pAttachments = &colour_attachment,
		  .subpassCount = 1,
		  .pSubpasses = &subpass,
		  .dependencyCount = 1,
		  .pDependencies = &subpass_dependency,
		};

		VkRenderPass resource;
		if (auto const result = vkCreateRenderPass(d.get(), &render_pass_info, allocator, &resource); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return render_pass(resource, d.get(), allocator);
	}

	render_pass::render_pass(VkRenderPass const renderpass, VkDevice const device, VkAllocationCallbacks const* const allocator) noexcept
	: render_pass_(renderpass, {vkDestroyRenderPass, device, allocator})
	{}

	error_or<pipeline_layout> pipeline_layout::create(device const& d, VkAllocationCallbacks const* const allocator) noexcept
	{
		constexpr auto layout_info = VkPipelineLayoutCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .setLayoutCount = 0,
		  .pSetLayouts = nullptr,
		  .pushConstantRangeCount = 0,
		  .pPushConstantRanges = nullptr,
		};

		auto layout = VkPipelineLayout{};
		if (auto const result = vkCreatePipelineLayout(d.get(), &layout_info, allocator, &layout); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return pipeline_layout(d.get(), layout, allocator);
	}

	pipeline_layout::pipeline_layout(
	  VkDevice const device,
	  VkPipelineLayout const layout,
	  VkAllocationCallbacks const* const allocator) noexcept
	: layout_(layout, {vkDestroyPipelineLayout, device, allocator})
	{}

	template<pipeline_kind kind>
	error_or<pipeline<kind>> pipeline<kind>::create(
	  device const& d,
	  pipeline_layout const& layout,
	  render_pass const& renderpass,
	  std::span<VkDynamicState const> const dynamic_states,
	  swapchain const& swap_chain,
	  std::span<vertex_shader const> vertex_shaders,
	  std::span<VkVertexInputBindingDescription const> const binding_descriptions,
	  std::span<VkVertexInputAttributeDescription const> const attribute_descriptions,
	  std::span<fragment_shader const> fragment_shaders,
	  std::span<tesselation_control_shader const> tesselation_control_shaders,
	  std::span<tesselation_evaluation_shader const> tesselation_evaluation_shaders,
	  std::span<geometry_shader const> geometry_shaders,
	  VkAllocationCallbacks const* const allocator) noexcept
	requires (kind == pipeline_kind::graphics)
	{
		auto const vertex_input_info = VkPipelineVertexInputStateCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .vertexBindingDescriptionCount = static_cast<std::uint32_t>(binding_descriptions.size()),
		  .pVertexBindingDescriptions = binding_descriptions.data(),
		  .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attribute_descriptions.size()),
		  .pVertexAttributeDescriptions = attribute_descriptions.data(),
		};
		constexpr auto input_assembly = VkPipelineInputAssemblyStateCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		  .primitiveRestartEnable = VK_FALSE,
		};
		auto const dynamic_state = VkPipelineDynamicStateCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
		  .pDynamicStates = dynamic_states.data(),
		};
		auto const viewport = VkViewport{
		  .x = 0.0f,
		  .y = 0.0f,
		  .width = static_cast<float>(swap_chain.extent().width),
		  .height = static_cast<float>(swap_chain.extent().height),
		  .minDepth = 0.0f,
		  .maxDepth = 1.0f,
		};
		auto const scissor = VkRect2D{
		  .offset = {0, 0},
		  .extent = swap_chain.extent(),
		};
		auto const viewport_state = VkPipelineViewportStateCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .viewportCount = 1,
		  .pViewports = &viewport,
		  .scissorCount = 1,
		  .pScissors = &scissor,
		};
		constexpr auto rasteriser = VkPipelineRasterizationStateCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .depthClampEnable = VK_FALSE,
		  .rasterizerDiscardEnable = VK_FALSE,
		  .polygonMode = VK_POLYGON_MODE_FILL,
		  .cullMode = VK_CULL_MODE_BACK_BIT,
		  .frontFace = VK_FRONT_FACE_CLOCKWISE,
		  .depthBiasEnable = VK_FALSE,
		  .depthBiasConstantFactor = 0.0f,
		  .depthBiasClamp = 0.0f,
		  .depthBiasSlopeFactor = 0.0f,
		  .lineWidth = 1.0f,
		};
		constexpr auto multisampling = VkPipelineMultisampleStateCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		  .sampleShadingEnable = VK_FALSE,
		  .minSampleShading = 1.0f,
		  .pSampleMask = nullptr,
		  .alphaToCoverageEnable = VK_FALSE,
		  .alphaToOneEnable = VK_FALSE,
		};
		constexpr auto colour_blend_attachment = VkPipelineColorBlendAttachmentState{
		  .blendEnable = VK_FALSE,
		  .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		  .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		  .colorBlendOp = VK_BLEND_OP_ADD,
		  .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		  .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		  .alphaBlendOp = VK_BLEND_OP_ADD,
		  .colorWriteMask =
		    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		};
		auto const colour_blending = VkPipelineColorBlendStateCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .logicOpEnable = VK_FALSE,
		  .logicOp = VK_LOGIC_OP_COPY,
		  .attachmentCount = 1,
		  .pAttachments = &colour_blend_attachment,
		  .blendConstants = {},
		};

		auto shader_stages = std::vector<VkPipelineShaderStageCreateInfo>(
		  vertex_shaders.size() + fragment_shaders.size() + tesselation_control_shaders.size()
		  + tesselation_evaluation_shaders.size() + geometry_shaders.size());
		auto pipeline_create_info = [](auto const& shader) noexcept { return shader.pipeline_create_info(); };
		auto next_shader = std::ranges::transform(vertex_shaders, shader_stages.begin(), pipeline_create_info).out;
		next_shader = std::ranges::transform(fragment_shaders, next_shader, pipeline_create_info).out;
		next_shader = std::ranges::transform(tesselation_control_shaders, next_shader, pipeline_create_info).out;
		next_shader = std::ranges::transform(tesselation_evaluation_shaders, next_shader, pipeline_create_info).out;
		std::ranges::transform(geometry_shaders, next_shader, pipeline_create_info);

		auto const pipeline_info = VkGraphicsPipelineCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .stageCount = static_cast<std::uint32_t>(shader_stages.size()),
		  .pStages = shader_stages.data(),
		  .pVertexInputState = &vertex_input_info,
		  .pInputAssemblyState = &input_assembly,
		  .pTessellationState = {},
		  .pViewportState = &viewport_state,
		  .pRasterizationState = &rasteriser,
		  .pMultisampleState = &multisampling,
		  .pDepthStencilState = nullptr,
		  .pColorBlendState = &colour_blending,
		  .pDynamicState = &dynamic_state,
		  .layout = layout.get(),
		  .renderPass = renderpass.get(),
		  .subpass = 0,
		  .basePipelineHandle = VK_NULL_HANDLE,
		  .basePipelineIndex = -1,
		};

		auto resource = VkPipeline{};
		if (auto const result = vkCreateGraphicsPipelines(d.get(), VK_NULL_HANDLE, 1, &pipeline_info, allocator, &resource);
		    result != VK_SUCCESS)
		{
			return std::unexpected(static_cast<error>(result));
		}

		return pipeline(resource, d.get(), allocator);
	}

	template<pipeline_kind kind>
	pipeline<kind>::pipeline(VkPipeline const p, VkDevice const d, VkAllocationCallbacks const* const a) noexcept
	: pipeline_(p, {vkDestroyPipeline, d, a})
	{}

	template<pipeline_kind kind>
	VkPipeline pipeline<kind>::get() const noexcept
	{
		return pipeline_.get();
	}

	template class pipeline<pipeline_kind::graphics>;
	template class pipeline<pipeline_kind::compute>;

	error_or<framebuffer> framebuffer::create(
	  device const& d,
	  image_view const& view,
	  render_pass const& pass,
	  swapchain const& chain,
	  VkAllocationCallbacks const* const alloc) noexcept
	{
		auto const attachment = view.get();
		auto const framebuffer_info = VkFramebufferCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		  .renderPass = pass.get(),
		  .attachmentCount = 1,
		  .pAttachments = &attachment,
		  .width = chain.extent().width,
		  .height = chain.extent().height,
		  .layers = 1,
		};

		auto resource = VkFramebuffer{};
		if (auto const result = vkCreateFramebuffer(d.get(), &framebuffer_info, alloc, &resource); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return framebuffer(resource, d.get(), alloc);
	}

	framebuffer::framebuffer(VkFramebuffer const buffer, VkDevice const device, VkAllocationCallbacks const* const alloc) noexcept
	: framebuffer_(buffer, {vkDestroyFramebuffer, device, alloc})
	{}

	VkFramebuffer framebuffer::get() const noexcept
	{
		return framebuffer_.get();
	}

	error_or<command_pool> command_pool::create(
	  device const& d,
	  window::window const& w,
	  VkAllocationCallbacks const* const alloc) noexcept
	{
		auto family = find_present_queue(d.physical_device(), w.get_surface());
		assert(family != std::nullopt);

		auto const pool_info = VkCommandPoolCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		  .queueFamilyIndex = *family,
		};

		auto resource = VkCommandPool{};
		if (auto const result = vkCreateCommandPool(d.get(), &pool_info, alloc, &resource); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return command_pool(resource, d.get(), alloc);
	}

	command_pool::command_pool(VkCommandPool const pool, VkDevice const d, VkAllocationCallbacks const* const alloc) noexcept
	: command_pool_(pool, {vkDestroyCommandPool, d, alloc})
	{}

	VkCommandPool command_pool::get() const noexcept
	{
		return command_pool_.get();
	}

	error_or<command_buffer> command_buffer::create(device const& d, command_pool const& p, std::uint32_t const size) noexcept
	{
		auto const buffer_info = VkCommandBufferAllocateInfo{
		  .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		  .pNext = nullptr,
		  .commandPool = p.get(),
		  .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		  .commandBufferCount = size,
		};

		auto resource = std::vector<VkCommandBuffer>(size);
		if (auto const result = vkAllocateCommandBuffers(d.get(), &buffer_info, resource.data()); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return command_buffer(std::move(resource));
	}

	command_buffer::command_buffer(std::vector<VkCommandBuffer> buffer) noexcept
	: buffer_(std::move(buffer))
	{}

	error_or<void> command_buffer::reset(std::uint32_t const frame) noexcept
	{
		if (auto const result = vkResetCommandBuffer(buffer_[frame], 0); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return {};
	}

	error_or<semaphore> semaphore::create(device const& d, VkAllocationCallbacks const* const alloc) noexcept
	{
		constexpr auto semaphore_info = VkSemaphoreCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = {},
		};

		auto resource = VkSemaphore{};
		if (auto const result = vkCreateSemaphore(d.get(), &semaphore_info, alloc, &resource); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return semaphore(resource, d.get(), alloc);
	}

	semaphore::semaphore(VkSemaphore const s, VkDevice const d, VkAllocationCallbacks const* const alloc) noexcept
	: semaphore_(s, {vkDestroySemaphore, d, alloc})
	{}

	error_or<fence> fence::create(device const& d, VkAllocationCallbacks const* const alloc) noexcept
	{
		constexpr auto fence_info = VkFenceCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		auto resource = VkFence{};
		if (auto const result = vkCreateFence(d.get(), &fence_info, alloc, &resource); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return fence(resource, d.get(), alloc);
	}

	fence::fence(VkFence const s, VkDevice const d, VkAllocationCallbacks const* const alloc) noexcept
	: fence_(s, {vkDestroyFence, d, alloc})
	, device_(d)
	{}

	error_or<void> present(
	  device const& d,
	  std::uint32_t const image_index,
	  std::span<VkSwapchainKHR const> const swapchains,
	  std::span<VkSemaphore const> const signals) noexcept
	{
		auto const present_info = VkPresentInfoKHR{
		  .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		  .pNext = nullptr,
		  .waitSemaphoreCount = static_cast<std::uint32_t>(signals.size()),
		  .pWaitSemaphores = signals.data(),
		  .swapchainCount = static_cast<std::uint32_t>(swapchains.size()),
		  .pSwapchains = swapchains.data(),
		  .pImageIndices = &image_index,
		  .pResults = nullptr,
		};

		if (auto const result = vkQueuePresentKHR(d.queue_, &present_info); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return {};
	}

	std::optional<std::uint32_t> find_memory_type(
	  std::uint32_t const filter,
	  VkMemoryPropertyFlags const properties,
	  VkPhysicalDeviceMemoryProperties const memory_properties) noexcept
	{
		for (auto i = std::uint32_t{0}; i < memory_properties.memoryTypeCount; ++i) {
			if (static_cast<bool>(filter & (1 << i))
			    and (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		return std::nullopt;
	}
} // namespace vulkan
