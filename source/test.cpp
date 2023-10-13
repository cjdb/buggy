#include <functional>
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#include <algorithm>
#include <array>
#include <buggy/vulkan.hpp>
#include <buggy/window.hpp>
#include <expected>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

struct panic {
	[[noreturn]] std::expected<void, vulkan::error> operator()(vulkan::error error)
	{
		switch (error) {
		case vulkan::error::no_host_memory:
			throw std::runtime_error("no host memory available");
		case vulkan::error::no_device_memory:
			throw std::runtime_error("no host device available");
		case vulkan::error::initialisation_failed:
			throw std::runtime_error("initialisation failed");
		case vulkan::error::device_lost:
			throw std::runtime_error("device lost");
		case vulkan::error::memory_map_failed:
			throw std::runtime_error("memory map failed");
		case vulkan::error::layer_unavailable:
			throw std::runtime_error("layer unavailable");
		case vulkan::error::extension_unavailable:
			throw std::runtime_error("extension unavailable");
		case vulkan::error::feature_unavailable:
			throw std::runtime_error("feature unavailable");
		case vulkan::error::incompatible_driver:
			throw std::runtime_error("incompatible driver");
		case vulkan::error::too_many_objects:
			throw std::runtime_error("too many objects");
		case vulkan::error::unsupported_format:
			throw std::runtime_error("unsupported format");
		case vulkan::error::fragmented_pool:
			throw std::runtime_error("fragmented pool");
		case vulkan::error::unknown:
			throw std::runtime_error("unknown error");
		case vulkan::error::no_pool_memory:
			throw std::runtime_error("no pool memory");
		case vulkan::error::no_suitable_devices:
			throw std::runtime_error("no suitable devices");
		case vulkan::error::file_not_found:
			throw std::runtime_error("file not found");
		case vulkan::error::timeout:
			throw std::runtime_error("timeout");
		case vulkan::error::out_of_date:
			throw std::runtime_error("out-of-date");
		default:
			break;
		}
		std::unreachable();
	}

	[[noreturn]] std::expected<void, window::error> operator()(window::error error)
	{
		switch (error) {
		case window::error::platform_error:
			throw std::runtime_error("a GLFW platform error occurred");
		default:
			std::unreachable();
		}
	}
};

enum class format : std::int8_t {
	// std::int8_t
	i8 = VK_FORMAT_R8_SINT,
	i8_vec2 = VK_FORMAT_R8G8_SINT,
	i8_vec3 = VK_FORMAT_R8G8B8_SINT,
	i8_vec4 = VK_FORMAT_R8G8B8A8_SINT,

	// std::uint8_t
	u8 = VK_FORMAT_R8_UINT,
	u8_vec2 = VK_FORMAT_R8G8_UINT,
	u8_vec3 = VK_FORMAT_R8G8B8_UINT,
	u8_vec4 = VK_FORMAT_R8G8B8A8_UINT,

	// std::int16_t
	i16 = VK_FORMAT_R16_SINT,
	i16_vec2 = VK_FORMAT_R16G16_SINT,
	i16_vec3 = VK_FORMAT_R16G16B16_SINT,
	i16_vec4 = VK_FORMAT_R16G16B16A16_SINT,

	// std::uint16_t
	u16 = VK_FORMAT_R16_UINT,
	u16_vec2 = VK_FORMAT_R16G16_UINT,
	u16_vec3 = VK_FORMAT_R16G16B16_UINT,
	u16_vec4 = VK_FORMAT_R16G16B16A16_UINT,

	// std::int32_t
	i32 = VK_FORMAT_R32_SINT,
	i32_vec2 = VK_FORMAT_R32G32_SINT,
	i32_vec3 = VK_FORMAT_R32G32B32_SINT,
	i32_vec4 = VK_FORMAT_R32G32B32A32_SINT,

	// std::uint32_t
	u32 = VK_FORMAT_R32_UINT,
	u32_vec2 = VK_FORMAT_R32G32_UINT,
	u32_vec3 = VK_FORMAT_R32G32B32_UINT,
	u32_vec4 = VK_FORMAT_R32G32B32A32_UINT,

	// std::int64_t
	i64 = VK_FORMAT_R64_SINT,
	i64_vec2 = VK_FORMAT_R64G64_SINT,
	i64_vec3 = VK_FORMAT_R64G64B64_SINT,
	i64_vec4 = VK_FORMAT_R64G64B64A64_SINT,

	// std::uint64_t
	u64 = VK_FORMAT_R64_UINT,
	u64_vec2 = VK_FORMAT_R64G64_UINT,
	u64_vec3 = VK_FORMAT_R64G64B64_UINT,
	u64_vec4 = VK_FORMAT_R64G64B64A64_UINT,

	// std::float16_t
	f16 = VK_FORMAT_R16_SFLOAT,
	f16_vec2 = VK_FORMAT_R16G16_SFLOAT,
	f16_vec3 = VK_FORMAT_R16G16B16_SFLOAT,
	f16_vec4 = VK_FORMAT_R16G16B16A16_SFLOAT,

	// std::float32_t
	f32 = VK_FORMAT_R32_SFLOAT,
	f32_vec2 = VK_FORMAT_R32G32_SFLOAT,
	f32_vec3 = VK_FORMAT_R32G32B32_SFLOAT,
	f32_vec4 = VK_FORMAT_R32G32B32A32_SFLOAT,

	// std::float64_t
	f64 = VK_FORMAT_R64_SFLOAT,
	f64_vec2 = VK_FORMAT_R64G64_SFLOAT,
	f64_vec3 = VK_FORMAT_R64G64B64_SFLOAT,
	f64_vec4 = VK_FORMAT_R64G64B64A64_SFLOAT,

	error,
};

template<class T>
consteval format get_format()
{
	return // std::int8_t
	  std::same_as<T, std::int8_t>     ? format::i8
	  : std::same_as<T, std::uint8_t>  ? format::u8
	  : std::same_as<T, std::int16_t>  ? format::i16
	  : std::same_as<T, std::uint16_t> ? format::u16
	  : std::same_as<T, std::int32_t>  ? format::i32
	  : std::same_as<T, glm::ivec2>    ? format::i32_vec2
	  : std::same_as<T, glm::ivec3>    ? format::i32_vec3
	  : std::same_as<T, glm::ivec4>    ? format::i32_vec4
	  : std::same_as<T, std::uint32_t> ? format::u32
	  : std::same_as<T, glm::uvec2>    ? format::u32_vec2
	  : std::same_as<T, glm::uvec3>    ? format::u32_vec3
	  : std::same_as<T, glm::uvec4>    ? format::u32_vec4
	  : std::same_as<T, std::int64_t>  ? format::i64
	  : std::same_as<T, std::uint64_t> ? format::u64
	  : std::same_as<T, float>         ? format::f32
	  : std::same_as<T, glm::vec2>     ? format::f32_vec2
	  : std::same_as<T, glm::vec3>     ? format::f32_vec3
	  : std::same_as<T, glm::vec4>     ? format::f32_vec4
	  : std::same_as<T, double>        ? format::f64
	  : std::same_as<T, glm::dvec2>    ? format::f32_vec2
	  : std::same_as<T, glm::dvec3>    ? format::f32_vec3
	  : std::same_as<T, glm::dvec4>
	    ? format::f64_vec4
	    : format::error;
}

template<class T>
constexpr VkVertexInputAttributeDescription make_attribute(
  std::uint32_t const binding,
  std::uint32_t const location,
  std::size_t const offset) noexcept
{
	return VkVertexInputAttributeDescription{
	  .location = location,
	  .binding = binding,
	  .format = static_cast<VkFormat>(get_format<T>()),
	  .offset = static_cast<std::uint32_t>(offset),
	};
}

struct vertex {
	glm::vec2 pos;
	glm::vec3 colour;

	static VkVertexInputBindingDescription const binding_description;
	static std::array<VkVertexInputAttributeDescription, 2> const attributes;
};

constexpr VkVertexInputBindingDescription vertex::binding_description = {
  .binding = 0,
  .stride = sizeof(vertex),
  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
};

constexpr std::array<VkVertexInputAttributeDescription, 2> vertex::attributes{
  make_attribute<glm::vec2>(0, 0, offsetof(vertex, pos)),
  make_attribute<glm::vec3>(0, 1, offsetof(vertex, colour)),
};

class hello_triangle_application {
public:
	void run()
	{
		auto frame = std::uint32_t{0};
		auto image_available = std::array{
		  *vulkan::semaphore::create(device_).transform_error(panic{}),
		  *vulkan::semaphore::create(device_).transform_error(panic{}),
		};
		auto render_finished = std::array{
		  *vulkan::semaphore::create(device_).transform_error(panic{}),
		  *vulkan::semaphore::create(device_).transform_error(panic{}),
		};
		auto frame_completed = std::array{
		  *vulkan::fence::create(device_).transform_error(panic{}),
		  *vulkan::fence::create(device_).transform_error(panic{}),
		};
		VkBuffer buffer[] = {buffer_.get()};

		auto reset_fences = [this, &frame, &frame_completed](std::uint32_t const i) -> vulkan::error_or<std::uint32_t> {
			VkFence fences[] = {frame_completed[frame].get()};
			if (auto result = device_.reset(fences); not result) {
				return std::unexpected(result.error());
			}

			return i;
		};
		auto reset_command_buffer = [&frame, this](std::uint32_t const i) -> vulkan::error_or<std::uint32_t> {
			if (auto const result = command_buffer_.reset(frame); not result) {
				return std::unexpected(result.error());
			}

			return i;
		};
		auto acquire_next_image = [this, &image_available, &frame] {
			return swapchain_.acquire_next_image(image_available[frame]);
		};
		auto record_command = [this, &frame, buffer](std::uint32_t const i) -> vulkan::error_or<std::uint32_t> {
			if (auto const result = command_buffer_.record(
			      frame,
			      i,
			      render_pass_,
			      swapchain_,
			      pipeline_,
			      framebuffer_,
			      [buffer](VkCommandBuffer const command_buffer) noexcept -> vulkan::error_or<void> {
				      VkDeviceSize offsets[] = {0};
				      vkCmdBindVertexBuffers(command_buffer, 0, 1, buffer, offsets);
				      vkCmdDraw(command_buffer, static_cast<std::uint32_t>(vertices.size()), 1, 0, 0);
				      return {};
			      });
			    not result)
			{
				return std::unexpected(result.error());
			}

			return i;
		};
		auto submit_command = [&, this](std::uint32_t const i) -> vulkan::error_or<std::uint32_t> {
			VkCommandBuffer commands[] = {command_buffer_.get(frame)};
			VkSemaphore wait[] = {image_available[frame].get()};
			VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
			VkSemaphore signal[] = {render_finished[frame].get()};
			if (auto const result = device_.submit(commands, wait, wait_stages, signal, frame_completed[frame]); not result) {
				return std::unexpected(result.error());
			}

			return i;
		};
		auto present = [this, &render_finished, &frame](std::uint32_t const image_index) {
			VkSwapchainKHR swapchains[] = {swapchain_.get()};
			VkSemaphore signal[] = {render_finished[frame].get()};
			return vulkan::present(device_, image_index, swapchains, signal);
		};
		auto recreate_swapchain = [this](vulkan::error const e) noexcept -> vulkan::error_or<void> {
			if (e == vulkan::error::out_of_date) {
				return std::unexpected(e);
			}

			return device_.wait().and_then([this]() -> vulkan::error_or<void> {
				auto result = vulkan::swapchain::create(device_, window_);
				if (result.has_value()) {
					swapchain_ = std::move(*result);
					return {};
				}

				return std::unexpected(result.error());
			});
		};

		while (not window_.should_close()) {
			VkFence current_frame[] = {frame_completed[frame].get()};
			(void)device_.wait_all({current_frame, 1})
			  .and_then(acquire_next_image)
			  .and_then(reset_fences)
			  .and_then(reset_command_buffer)
			  .and_then(record_command)
			  .and_then(submit_command)
			  .and_then(present)
			  .or_else(recreate_swapchain)
			  .transform_error(panic{});
			glfwPollEvents();
			frame = (frame + 1) % 2;
		}

		(void)device_.wait().transform_error(panic{});
	}
private:
	static inline constexpr auto width = 800u;
	static inline constexpr auto height = 600u;
	static inline constexpr auto layers = std::array{
	  "VK_LAYER_KHRONOS_validation",
	};
	vulkan::instance instance_ = [] {
		VkApplicationInfo app_info{
		  .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		  .pNext = nullptr,
		  .pApplicationName = "Hello Triangle",
		  .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		  .pEngineName = "No Engine",
		  .engineVersion = VK_MAKE_VERSION(1, 0, 0),
		  .apiVersion = VK_API_VERSION_1_0,
		};

		char const* const debug = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		return *vulkan::instance::create(app_info, layers, std::span{&debug, 1}).transform_error(panic{});
	}();
	vulkan::debug_utils debug_messenger_ = [this] {
		using vulkan::debug_utils;
		return *debug_utils::create(
		          instance_.get(),
		          debug_utils::verbose | debug_utils::warning | debug_utils::error,
		          debug_utils::general | debug_utils::validation | debug_utils::performance)
		          .transform_error(panic{});
	}();
	window::window window_ = *window::window::create(instance_, {width, height}, "test", window::window::fullscreen::no);
	vulkan::device device_ = [this] {
		constexpr auto extensions = std::array{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		return *vulkan::device::create(instance_, window_, [](auto const&) noexcept { return true; }, extensions).transform_error(panic{});
	}();
	vulkan::swapchain swapchain_ = *vulkan::swapchain::create(device_, window_).transform_error(panic{});
	vulkan::render_pass render_pass_ = *vulkan::render_pass::create(device_, swapchain_).transform_error(panic{});
	vulkan::pipeline_layout pipeline_layout_ = *vulkan::pipeline_layout::create(device_).transform_error(panic{});
	vulkan::graphics_pipeline pipeline_ = [this] {
		auto dynamic_states = std::array{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		auto const vertex_shader =
		  *vulkan::vertex_shader::create("/home/cjdb/projects/buggy/vert.spv", device_).transform_error(panic{});
		auto const fragment_shader =
		  *vulkan::fragment_shader::create("/home/cjdb/projects/buggy/frag.spv", device_).transform_error(panic{});
		return *vulkan::graphics_pipeline::create(
		          device_,
		          pipeline_layout_,
		          render_pass_,
		          dynamic_states,
		          swapchain_,
		          {&vertex_shader, 1},
		          {&vertex::binding_description, 1},
		          vertex::attributes,
		          {&fragment_shader, 1})
		          .transform_error(panic{});
	}();
	std::vector<vulkan::framebuffer> framebuffer_ = [this] {
		auto result = std::vector<vulkan::framebuffer>();
		result.reserve(swapchain_.size());
		std::ranges::transform(
		  swapchain_.image_views(),
		  std::back_inserter(result),
		  [this](vulkan::image_view const& image_view) noexcept {
			  return *vulkan::framebuffer::create(device_, image_view, render_pass_, swapchain_).transform_error(panic{});
		  });
		return result;
	}();
	vulkan::command_pool command_pool_ = *vulkan::command_pool::create(device_, window_).transform_error(panic{});
	static inline std::vector<vertex> const vertices = {
	  {{0.0f, -0.5f}, {1.0f, 0.25f, 0.25f}},
	  { {0.5f, 0.5f}, {0.25f, 1.0f, 0.25f}},
	  {{-0.5f, 0.5f}, {0.25f, 0.25f, 1.0f}}
  };
	vulkan::buffer<vertex> buffer_ = *vulkan::buffer<vertex>::create(device_, vertices).transform_error(panic{});
	vulkan::command_buffer command_buffer_ =
	  *vulkan::command_buffer::create(device_, command_pool_, 2).transform_error(panic{});
};

int main()
{
	try {
		(void)window::context::create().or_else(panic{});
		hello_triangle_application app;
		app.run();
	}
	catch (std::exception const& e) {
		std::cerr << e.what() << '\n';
		return 1;
	}

	return 0;
}
