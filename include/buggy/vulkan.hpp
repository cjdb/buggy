#ifndef BUGGY_VULKAN_ERROR_HPP
#define BUGGY_VULKAN_ERROR_HPP

#include <algorithm>
#include <cstdint>
#include <expected>
#include <memory>
#include <numeric>
#include <optional>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace window {
	class window;
}

namespace vulkan {
	enum class error : std::int32_t {
		// Vulkan errors
		no_host_memory = VK_ERROR_OUT_OF_HOST_MEMORY,
		no_device_memory = VK_ERROR_OUT_OF_DEVICE_MEMORY,
		initialisation_failed = VK_ERROR_INITIALIZATION_FAILED,
		device_lost = VK_ERROR_DEVICE_LOST,
		memory_map_failed = VK_ERROR_MEMORY_MAP_FAILED,
		layer_unavailable = VK_ERROR_LAYER_NOT_PRESENT,
		extension_unavailable = VK_ERROR_EXTENSION_NOT_PRESENT,
		feature_unavailable = VK_ERROR_FEATURE_NOT_PRESENT,
		incompatible_driver = VK_ERROR_INCOMPATIBLE_DRIVER,
		too_many_objects = VK_ERROR_TOO_MANY_OBJECTS,
		unsupported_format = VK_ERROR_FORMAT_NOT_SUPPORTED,
		fragmented_pool = VK_ERROR_FRAGMENTED_POOL,
		unknown = VK_ERROR_UNKNOWN,
		no_pool_memory = VK_ERROR_OUT_OF_POOL_MEMORY,
		invalid_external_handle = VK_ERROR_INVALID_EXTERNAL_HANDLE,
		fragmentation = VK_ERROR_FRAGMENTATION,
		invalid_opaque_capture_address = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
		surface_lost = VK_ERROR_SURFACE_LOST_KHR,
		native_window_in_use = VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
		out_of_date = VK_ERROR_OUT_OF_DATE_KHR,
		incompatible_display = VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,
		validation_failed = VK_ERROR_VALIDATION_FAILED_EXT,
		// invalid_shader_nv = VK_ERROR_INVALID_SHADER_NV,
		unsupported_image_usage = VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR,
		unsupported_video_layout = VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR,
		unsupported_video_operation = VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR,
		unsupported_video_profile_format = VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR,
		unsupported_video_profile_codec = VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR,
		unsupported_video_std_version = VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR,
		unsupported_drm_format = VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT,
		not_permitted = VK_ERROR_NOT_PERMITTED_KHR,
		lost_fullscreen_exclusivity = VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,

		// API errors
		no_suitable_devices = 1,
		file_not_found,
		timeout,
	};

	template<class T>
	using error_or = std::expected<T, error>;

	struct no_owner {};

	template<class F, class Owner = no_owner>
	class deleter {
	public:
		deleter(F f, VkAllocationCallbacks const* const allocator) noexcept
		requires std::same_as<Owner, no_owner>
		: deleter_(std::move(f))
		, allocator_(allocator)
		{}

		deleter(F f, Owner owner, VkAllocationCallbacks const* const allocator) noexcept
		: deleter_(std::move(f))
		, owner_(std::move(owner))
		, allocator_(allocator)
		{}

		template<class T>
		requires std::invocable<F, T, VkAllocationCallbacks const*>
		void operator()(T const t) const noexcept
		{
			deleter_(t, allocator_);
		}

		template<class T>
		requires std::invocable<F, Owner, T, VkAllocationCallbacks const*>
		void operator()(T const t) const noexcept
		{
			if (t) {
				deleter_(owner_, t, allocator_);
			}
		}
	private:
		F deleter_;
		[[no_unique_address]] Owner owner_;
		VkAllocationCallbacks const* allocator_;
	};

	struct physical_device {
		VkPhysicalDevice device;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceMemoryProperties memory_properties;
		std::vector<VkExtensionProperties> extensions;
	};

	class instance {
	public:
		[[nodiscard]] static error_or<instance> create(
		  VkApplicationInfo app_info,
		  std::span<char const* const> layers,
		  std::span<char const* const> extensions) noexcept;

		[[nodiscard]] static error_or<instance> create(
		  VkApplicationInfo app_info,
		  VkAllocationCallbacks const* allocator,
		  std::span<char const* const> layers,
		  std::span<char const* const> extensions) noexcept;

		[[nodiscard]] std::span<physical_device const> physical_devices() const noexcept;

		[[nodiscard]] VkInstance get() const noexcept
		{
			return instance_.get();
		}
	private:
		std::unique_ptr<VkInstance_T, deleter<PFN_vkDestroyInstance>> instance_;
		std::vector<physical_device> physical_devices_;

		instance(VkInstance instance, VkAllocationCallbacks const* allocator) noexcept;
		static std::vector<physical_device> retrieve_devices(VkInstance instance) noexcept;
	};

	class debug_utils {
	public:
		enum class severity_t : std::uint16_t {
			verbose = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
			info = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
			warning = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
			error = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		};
		using enum severity_t;

		[[nodiscard]] friend severity_t operator|(severity_t const x, severity_t const y) noexcept
		{
			return static_cast<severity_t>(static_cast<std::uint32_t>(x) | static_cast<std::uint32_t>(y));
		}

		enum class type_t : std::uint8_t {
			general = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
			validation = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
			performance = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		};
		using enum type_t;

		[[nodiscard]] friend type_t operator|(type_t const x, type_t const y) noexcept
		{
			return static_cast<type_t>(static_cast<std::uint32_t>(x) | static_cast<std::uint32_t>(y));
		}

		using diagnostic_callback_t = PFN_vkDebugUtilsMessengerCallbackEXT;

		[[nodiscard]] static error_or<debug_utils> create(
		  VkInstance instance,
		  severity_t severity,
		  type_t type,
		  diagnostic_callback_t callback = log,
		  VkAllocationCallbacks const* allocator = nullptr) noexcept;

		friend class instance;
	private:
		using handler = std::unique_ptr<VkDebugUtilsMessengerEXT_T, deleter<PFN_vkDestroyDebugUtilsMessengerEXT, VkInstance>>;
		handler messenger_;

		debug_utils(VkDebugUtilsMessengerEXT, VkInstance, VkAllocationCallbacks const*) noexcept;

		static VKAPI_ATTR VkBool32 VKAPI_CALL log(
		  VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		  VkDebugUtilsMessageTypeFlagsEXT message_type,
		  VkDebugUtilsMessengerCallbackDataEXT const* message_data,
		  void* user_data) noexcept;
	};

	class fence;
	class semaphore;
	class swapchain;

	class device {
	public:
		using selector_fn = bool (*)(physical_device const&) noexcept;

		[[nodiscard]] static error_or<device> create(
		  instance const& instance,
		  window::window const& window,
		  selector_fn selector,
		  std::span<char const* const> extensions = {},
		  VkAllocationCallbacks const* allocator = nullptr) noexcept;

		[[nodiscard]] VkDevice get() const noexcept
		{
			return device_.get();
		}

		[[nodiscard]] physical_device const& physical_device() const noexcept
		{
			return *physical_device_;
		}

		[[nodiscard]] error_or<void> wait_one(
		  std::span<VkFence const> fences,
		  std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max()) noexcept;
		[[nodiscard]] error_or<void> wait_all(
		  std::span<VkFence const> fences,
		  std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max()) noexcept;
		[[nodiscard]] error_or<void> reset(std::span<VkFence const> fences) noexcept;

		[[nodiscard]] error_or<void> submit(
		  std::span<VkCommandBuffer> commands,
		  std::span<VkSemaphore> wait,
		  std::span<VkPipelineStageFlags const> wait_stages,
		  std::span<VkSemaphore> signals,
		  fence& f) noexcept;

		[[nodiscard]] error_or<void> wait() const noexcept
		{
			if (auto const result = vkDeviceWaitIdle(device_.get()); result != VK_SUCCESS) {
				return std::unexpected(static_cast<error>(result));
			}

			return {};
		}

		friend error_or<void> present(
		  device const& d,
		  std::uint32_t image_index,
		  std::span<VkSwapchainKHR const> swapchains,
		  std::span<VkSemaphore const> signals) noexcept;
	private:
		std::unique_ptr<VkDevice_T, deleter<PFN_vkDestroyDevice>> device_;
		VkQueue queue_;
		struct physical_device const* physical_device_;

		explicit device(VkDevice, VkQueue, VkAllocationCallbacks const*, struct physical_device const&) noexcept;
	};

	class image_view {
	public:
		[[nodiscard]] static error_or<image_view> create(
		  device const& d,
		  VkImage image,
		  VkFormat format,
		  VkAllocationCallbacks const* allocator = nullptr) noexcept;

		[[nodiscard]] VkImageView get() const noexcept;
	private:
		std::unique_ptr<VkImageView_T, deleter<PFN_vkDestroyImageView, VkDevice>> view_;

		image_view(VkImageView, VkDevice, VkAllocationCallbacks const*) noexcept;
	};

	class semaphore;

	class swapchain {
	public:
		[[nodiscard]] static error_or<swapchain> create(
		  device const& d,
		  window::window const& w,
		  VkAllocationCallbacks const* allocator = nullptr) noexcept;

		[[nodiscard]] VkSwapchainKHR get() const noexcept
		{
			return swapchain_.get();
		}

		[[nodiscard]] VkFormat format() const noexcept;
		[[nodiscard]] VkExtent2D extent() const noexcept;
		[[nodiscard]] std::uint32_t size() const noexcept;
		[[nodiscard]] std::span<image_view const> image_views() const;

		[[nodiscard]] error_or<std::uint32_t> acquire_next_image(
		  semaphore& sema,
		  std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max()) noexcept;
	private:
		std::unique_ptr<VkSwapchainKHR_T, deleter<PFN_vkDestroySwapchainKHR, VkDevice>> swapchain_;
		std::vector<VkImage> images_;
		VkFormat format_;
		VkExtent2D extent_;
		std::vector<image_view> image_views_;
		VkDevice device_;

		explicit swapchain(VkSwapchainKHR, device const&, VkAllocationCallbacks const*, VkFormat format, VkExtent2D extent) noexcept;
	};

	[[nodiscard]] error_or<void> present(
	  device const& d,
	  std::uint32_t image_index,
	  std::span<VkSwapchainKHR const> swapchains,
	  std::span<VkSemaphore const> signals) noexcept;

	template<VkShaderStageFlagBits>
	class shader_module {
	public:
		[[nodiscard]] static error_or<shader_module> create(
		  std::string_view path,
		  device const& d,
		  VkAllocationCallbacks const* allocator = nullptr) noexcept;

		[[nodiscard]] VkPipelineShaderStageCreateInfo pipeline_create_info(
		  std::string_view entry_point_name = "main") const noexcept;
	private:
		using handler = std::unique_ptr<VkShaderModule_T, deleter<PFN_vkDestroyShaderModule, VkDevice>>;
		handler module_;

		explicit shader_module(VkShaderModule, VkDevice, VkAllocationCallbacks const*) noexcept;
	};

	extern template class shader_module<VK_SHADER_STAGE_VERTEX_BIT>;
	using vertex_shader = shader_module<VK_SHADER_STAGE_VERTEX_BIT>;

	extern template class shader_module<VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT>;
	using tesselation_control_shader = shader_module<VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT>;

	extern template class shader_module<VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT>;
	using tesselation_evaluation_shader = shader_module<VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT>;

	extern template class shader_module<VK_SHADER_STAGE_GEOMETRY_BIT>;
	using geometry_shader = shader_module<VK_SHADER_STAGE_GEOMETRY_BIT>;

	extern template class shader_module<VK_SHADER_STAGE_FRAGMENT_BIT>;
	using fragment_shader = shader_module<VK_SHADER_STAGE_FRAGMENT_BIT>;

	extern template class shader_module<VK_SHADER_STAGE_COMPUTE_BIT>;
	using compute_shader = shader_module<VK_SHADER_STAGE_COMPUTE_BIT>;

	class render_pass {
	public:
		[[nodiscard]] static error_or<render_pass> create(
		  device const& d,
		  swapchain const& s,
		  VkAllocationCallbacks const* allocator = nullptr) noexcept;

		[[nodiscard]] VkRenderPass get() const noexcept
		{
			return render_pass_.get();
		}
	private:
		std::unique_ptr<VkRenderPass_T, deleter<PFN_vkDestroyRenderPass, VkDevice>> render_pass_;

		render_pass(VkRenderPass, VkDevice, VkAllocationCallbacks const*) noexcept;
	};

	class pipeline_layout {
	public:
		[[nodiscard]] static error_or<pipeline_layout> create(
		  device const& d,
		  VkAllocationCallbacks const* allocator = nullptr) noexcept;

		[[nodiscard]] VkPipelineLayout get() const noexcept
		{
			return layout_.get();
		}
	private:
		std::unique_ptr<VkPipelineLayout_T, deleter<PFN_vkDestroyPipelineLayout, VkDevice>> layout_;

		pipeline_layout(VkDevice, VkPipelineLayout, VkAllocationCallbacks const*) noexcept;
	};

	enum class pipeline_kind : std::uint8_t { graphics, compute };

	template<pipeline_kind kind>
	class pipeline {
	public:
		[[nodiscard]] static error_or<pipeline> create(
		  device const& d,
		  pipeline_layout const& layout,
		  render_pass const& renderpass,
		  std::span<VkDynamicState const> dynamic_states,
		  swapchain const& swap_chain,
		  std::span<vertex_shader const> vertex_shaders,
		  std::span<VkVertexInputBindingDescription const> binding_descriptions,
		  std::span<VkVertexInputAttributeDescription const> attribute_descriptions,
		  std::span<fragment_shader const> fragment_shaders,
		  std::span<tesselation_control_shader const> tesselation_control_shaders = {},
		  std::span<tesselation_evaluation_shader const> tesselation_evaluation_shaders = {},
		  std::span<geometry_shader const> geometry_shaders = {},
		  VkAllocationCallbacks const* allocator = nullptr) noexcept
		requires (kind == pipeline_kind::graphics);

		[[nodiscard]] static error_or<pipeline> create(std::span<compute_shader const> kernels) noexcept
		requires (kind == pipeline_kind::compute);

		[[nodiscard]] VkPipeline get() const noexcept;
	private:
		std::unique_ptr<VkPipeline_T, deleter<PFN_vkDestroyPipeline, VkDevice>> pipeline_;

		pipeline(VkPipeline, VkDevice, VkAllocationCallbacks const*) noexcept;
	};

	extern template class pipeline<pipeline_kind::graphics>;
	using graphics_pipeline = pipeline<pipeline_kind::graphics>;

	extern template class pipeline<pipeline_kind::compute>;
	using compute_pipeline = pipeline<pipeline_kind::compute>;

	class framebuffer {
	public:
		[[nodiscard]] static error_or<framebuffer> create(
		  device const& d,
		  image_view const& view,
		  render_pass const& pass,
		  swapchain const& chain,
		  VkAllocationCallbacks const* alloc = nullptr) noexcept;

		[[nodiscard]] VkFramebuffer get() const noexcept;
	private:
		std::unique_ptr<VkFramebuffer_T, deleter<PFN_vkDestroyFramebuffer, VkDevice>> framebuffer_;

		framebuffer(VkFramebuffer, VkDevice, VkAllocationCallbacks const*) noexcept;
	};

	class command_pool {
	public:
		static error_or<command_pool> create(
		  device const& d,
		  window::window const& w,
		  VkAllocationCallbacks const* alloc = nullptr) noexcept;

		[[nodiscard]] VkCommandPool get() const noexcept;
	private:
		std::unique_ptr<VkCommandPool_T, deleter<PFN_vkDestroyCommandPool, VkDevice>> command_pool_;

		command_pool(VkCommandPool, VkDevice, VkAllocationCallbacks const*) noexcept;
	};

	class command_buffer {
	public:
		[[nodiscard]] static error_or<command_buffer> create(
		  device const& d,
		  command_pool const& p,
		  std::uint32_t max_commands) noexcept;

		[[nodiscard]] VkCommandBuffer get(std::uint32_t const frame) const noexcept
		{
			return buffer_[frame];
		}

		template<std::invocable<VkCommandBuffer> F>
		requires std::same_as<std::invoke_result_t<F, VkCommandBuffer>, error_or<void>>
		[[nodiscard]] error_or<void> record(
		  std::uint32_t const frame,
		  std::uint32_t const image_index,
		  render_pass const& pass,
		  swapchain const& chain,
		  graphics_pipeline const& pipeline,
		  std::span<framebuffer const> const buffers,
		  F custom_op) noexcept
		{
			constexpr auto begin_info = VkCommandBufferBeginInfo{
			  .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			  .pNext = nullptr,
			  .flags = {},
			  .pInheritanceInfo = nullptr,
			};

			if (auto const result = vkBeginCommandBuffer(buffer_[frame], &begin_info); result != VK_SUCCESS) {
				return std::unexpected(static_cast<error>(result));
			}

			auto const render_area = VkRect2D{
			  .offset = {},
			  .extent = chain.extent(),
			};
			auto const clear_colour = VkClearValue{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}};
			auto const render_pass_info = VkRenderPassBeginInfo{
			  .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			  .pNext = nullptr,
			  .renderPass = pass.get(),
			  .framebuffer = buffers[image_index].get(),
			  .renderArea = render_area,
			  .clearValueCount = 1,
			  .pClearValues = &clear_colour,
			};

			vkCmdBeginRenderPass(buffer_[frame], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(buffer_[frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get());

			auto const viewport = VkViewport{
			  .x = 0.0f,
			  .y = 0.0f,
			  .width = static_cast<float>(chain.extent().width),
			  .height = static_cast<float>(chain.extent().height),
			  .minDepth = 0.0f,
			  .maxDepth = 1.0f,
			};
			vkCmdSetViewport(buffer_[frame], 0, 1, &viewport);
			vkCmdSetScissor(buffer_[frame], 0, 1, &render_area);
			if (auto const result = custom_op(buffer_[frame]); not result.has_value()) {
				return result;
			}
			vkCmdEndRenderPass(buffer_[frame]);
			if (auto const result = vkEndCommandBuffer(buffer_[frame]); result != VK_SUCCESS) {
				return std::unexpected(static_cast<error>(result));
			}

			return {};
		}

		[[nodiscard]] error_or<void> reset(std::uint32_t frame) noexcept;
	private:
		std::vector<VkCommandBuffer> buffer_;

		explicit command_buffer(std::vector<VkCommandBuffer>) noexcept;
	};

	class semaphore {
	public:
		[[nodiscard]] static error_or<semaphore> create(device const& d, VkAllocationCallbacks const* alloc = nullptr) noexcept;

		[[nodiscard]] VkSemaphore get() const noexcept
		{
			return semaphore_.get();
		}
	private:
		std::unique_ptr<VkSemaphore_T, deleter<PFN_vkDestroySemaphore, VkDevice>> semaphore_;

		semaphore(VkSemaphore, VkDevice, VkAllocationCallbacks const*) noexcept;
	};

	class fence {
	public:
		[[nodiscard]] static error_or<fence> create(device const& d, VkAllocationCallbacks const* alloc = nullptr) noexcept;

		[[nodiscard]] VkFence get() const noexcept
		{
			return fence_.get();
		}

		friend class device;
	private:
		std::unique_ptr<VkFence_T, deleter<PFN_vkDestroyFence, VkDevice>> fence_;
		VkDevice device_;

		fence(VkFence, VkDevice, VkAllocationCallbacks const*) noexcept;
	};

	[[nodiscard]] std::optional<std::uint32_t> find_memory_type(
	  std::uint32_t filter,
	  VkMemoryPropertyFlags properties,
	  VkPhysicalDeviceMemoryProperties memory_properties) noexcept;

	template<class T>
	requires std::is_standard_layout_v<T> and std::is_trivially_copyable_v<T>
	class buffer {
		using buffer_handler = std::unique_ptr<VkBuffer_T, deleter<PFN_vkDestroyBuffer, VkDevice>>;
		using memory_handler = std::unique_ptr<VkDeviceMemory_T, deleter<PFN_vkFreeMemory, VkDevice>>;
	public:
		[[nodiscard]] static error_or<buffer> create(
		  device const& d,
		  VkBufferUsageFlags const usage,
		  VkMemoryPropertyFlags properties,
		  VkDeviceSize size,
		  VkAllocationCallbacks const* alloc = nullptr) noexcept
		{
			auto const buffer_info = VkBufferCreateInfo{
			  .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			  .pNext = nullptr,
			  .flags = {},
			  .size = size,
			  .usage = usage,
			  .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			  .queueFamilyIndexCount = {},
			  .pQueueFamilyIndices = {},
			};

			auto buffer_resource = VkBuffer{};
			if (auto const result = vkCreateBuffer(d.get(), &buffer_info, alloc, &buffer_resource); result != VK_SUCCESS) {
				return std::unexpected(static_cast<error>(result));
			}

			auto b = buffer_handler(buffer_resource, {vkDestroyBuffer, d.get(), alloc});

			auto memory_requirements = VkMemoryRequirements{};
			vkGetBufferMemoryRequirements(d.get(), buffer_resource, &memory_requirements);
			auto const memory_type =
			  find_memory_type(memory_requirements.memoryTypeBits, properties, d.physical_device().memory_properties);
			if (memory_type == std::nullopt) {
				return std::unexpected(error::no_device_memory);
			}

			auto const alloc_info = VkMemoryAllocateInfo{
			  .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			  .pNext = nullptr,
			  .allocationSize = memory_requirements.size,
			  .memoryTypeIndex = *memory_type,
			};

			auto memory_resource = VkDeviceMemory{};
			if (auto const result = vkAllocateMemory(d.get(), &alloc_info, alloc, &memory_resource); result != VK_SUCCESS) {
				return std::unexpected(static_cast<error>(result));
			}

			auto m = memory_handler(memory_resource, {vkFreeMemory, d.get(), alloc});
			if (auto const result = vkBindBufferMemory(d.get(), buffer_resource, memory_resource, 0); result != VK_SUCCESS) {
				return std::unexpected(static_cast<error>(result));
			}

			return buffer{std::move(b), std::move(m)};
		}

		[[nodiscard]] error_or<buffer> create(
		  device const& d,
		  std::span<T const> data,
		  VkAllocationCallbacks const* alloc = nullptr) noexcept
		{
			auto const size = static_cast<VkDeviceSize>(sizeof(T) * data.size());
			return create(d,
			              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT data,
			              alloc)
			  .and_then([this, &d, data, alloc](buffer b) -> error_or<buffer> {
				  auto device_data = static_cast<void*>(nullptr);
				  if (auto const result = vkMapMemory(d.get(), b, 0, size, 0, &device_data); result != VK_SUCCESS) {
					  return std::unexpected(static_cast<error>(result));
				  }

				  std::memcpy(device_data, data.data(), size);
				  vkUnmapMemory(d.get(), b.device_memory_);
				  return b;
			  })
			  .and_then([this, &d, alloc](buffer source) noexcept -> error_or<std::pair<buffer, buffer>> {
				  auto dest = create(
				    d,
				    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				    size,
				    alloc);
				  if (not dest) {
					  return std::unexpected(dest.error());
				  }

				  return std::make_pair(std::move(source, std::move(*dest)));
			  })
			  .and_then([](std::pair<buffer, buffer> data) -> error_or<buffer> {

			  });
		}

		[[nodiscard]] VkBuffer get() const noexcept
		{
			return buffer_.get();
		}
	private:
		buffer_handler buffer_;
		memory_handler device_memory_;

		buffer(buffer_handler b, memory_handler m) noexcept
		: buffer_(std::move(b))
		, device_memory_(std::move(m))
		{}
	};
} // namespace vulkan

#endif // BUGGY_VULKAN_ERROR_HPP
