#ifndef BUGGY_GLFW_HPP
#define BUGGY_GLFW_HPP

#include "vulkan.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <expected>
#include <glm/vec2.hpp>
#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace window {
	enum class error {
		not_initialised = GLFW_NOT_INITIALIZED,
		no_current_context = GLFW_NO_CURRENT_CONTEXT,
		invalid_enum = GLFW_INVALID_ENUM,
		invalid_value = GLFW_INVALID_VALUE,
		out_of_memory = GLFW_OUT_OF_MEMORY,
		api_unavailable = GLFW_API_UNAVAILABLE,
		version_unavailable = GLFW_VERSION_UNAVAILABLE,
		platform_error = GLFW_PLATFORM_ERROR,
		format_unavailable = GLFW_FORMAT_UNAVAILABLE,
		no_window_context = GLFW_NO_WINDOW_CONTEXT,
	};

	class context {
	public:
		[[nodiscard]] static std::expected<void, error> create(GLFWerrorfun error_callback = log_error) noexcept;

		context(context&&) = delete;
		context& operator=(context&&) = delete;
		context(context const&) = delete;
		context& operator=(context const&) = delete;

		[[nodiscard]] static std::span<char const* const> required_extensions() noexcept;
	private:
		context() = default;
		~context();

		static void log_error(int code, char const* message) noexcept;
	};

	class window {
	public:
		enum class fullscreen : std::int8_t { no, yes };
		enum class focus : std::int8_t { no, yes };

		[[nodiscard]] static std::expected<window, error> create(
		  vulkan::instance const& instance,
		  glm::ivec2 dimensions,
		  std::string_view title,
		  fullscreen make_fullscreen,
		  focus is_focussed = focus::yes,
		  VkAllocationCallbacks const* allocator = nullptr) noexcept;

		[[nodiscard]] glm::ivec2 dimensions() const noexcept;
		[[nodiscard]] bool should_close() const noexcept;
		[[nodiscard]] std::expected<void, error>
		resize(glm::ivec2 dimensions, fullscreen make_fullscreen, focus is_focussed = focus::yes) noexcept;

		[[nodiscard]] GLFWwindow* get_window() const noexcept
		{
			return window_.get();
		}

		[[nodiscard]] VkSurfaceKHR get_surface() const noexcept
		{
			return surface_.get();
		}
	private:
		using window_handler = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow*)>;
		window_handler window_;

		using surface_handler = std::unique_ptr<VkSurfaceKHR_T, vulkan::deleter<PFN_vkDestroySurfaceKHR, VkInstance>>;
		surface_handler surface_;

		window(window_handler, surface_handler) noexcept;
	};
} // namespace window

#endif // BUGGY_GLFW_HPP
