#include <GLFW/glfw3.h>
#include <buggy/window.hpp>
#include <expected>
#include <print>
#include <vector>

namespace window {
	std::expected<void, error> context::create(GLFWerrorfun const error_callback) noexcept
	{
		glfwSetErrorCallback(error_callback);
		static auto _ = context{};
		auto const status = glfwInit();
		if (status == GLFW_FALSE) {
			return std::unexpected(error::platform_error);
		}

		return {};
	}

	context::~context()
	{
		glfwTerminate();
	}

	std::span<char const* const> context::required_extensions() noexcept
	{
		static std::vector<char const*> const extensions = []() noexcept {
			auto extension_count = std::uint32_t{0};
			auto glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);
			return std::vector<char const*>(glfw_extensions, glfw_extensions + extension_count);
		}();

		return extensions;
	}

	void context::log_error(int const code, char const* const message) noexcept
	{
		std::println(stderr, "GLFW error: {}: {}\n", code, message);
	}

	window::window(std::unique_ptr<GLFWwindow, void (*)(GLFWwindow*)> w,
	               std::unique_ptr<VkSurfaceKHR_T, vulkan::deleter<PFN_vkDestroySurfaceKHR, VkInstance>> s) noexcept
	: window_(std::move(w))
	, surface_(std::move(s))
	{}

	std::expected<window, error> window::create(
	  vulkan::instance const& instance,
	  glm::ivec2 const dimensions,
	  std::string_view const title,
	  fullscreen const make_fullscreen,
	  focus const is_focussed,
	  VkAllocationCallbacks const* allocator) noexcept
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		auto window_handle = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow*)>{
		  glfwCreateWindow(
		    dimensions.x,
		    dimensions.y,
		    title.data(),
		    make_fullscreen == fullscreen::yes ? glfwGetPrimaryMonitor() : nullptr,
		    nullptr),
		  glfwDestroyWindow,
		};

		if (window_handle == nullptr) {
			return std::unexpected(static_cast<error>(glfwGetError(nullptr)));
		}

		if (is_focussed == focus::yes) {
			glfwFocusWindow(window_handle.get());
		}

		auto surface = VkSurfaceKHR{};
		if (glfwCreateWindowSurface(instance.get(), window_handle.get(), allocator, &surface) != VK_SUCCESS) {
			std::print(stderr, "glfwCreateWindowSurface should never fail\n");
			std::abort();
		}

		return window(
		  std::move(window_handle),
		  surface_handler{
		    surface,
		    {vkDestroySurfaceKHR, instance.get(), allocator},
    });
	}

	glm::ivec2 window::dimensions() const noexcept
	{
		auto result = glm::ivec2{};
		glfwGetWindowSize(window_.get(), &result.x, &result.y);
		return result;
	}

	bool window::should_close() const noexcept
	{
		return static_cast<bool>(glfwWindowShouldClose(window_.get()));
	}

	std::expected<void, error>
	window::resize(glm::ivec2 const dimensions, fullscreen const make_fullscreen, focus const is_focussed) noexcept
	{
		if (make_fullscreen == fullscreen::yes) {
			glfwSetWindowMonitor(window_.get(), glfwGetPrimaryMonitor(), 0, 0, dimensions.x, dimensions.y, GLFW_DONT_CARE);
		}
		else {
			glfwSetWindowSize(window_.get(), dimensions.x, dimensions.y);
		}

		if (auto const error_code = glfwGetError(nullptr); error_code != GLFW_NO_ERROR) {
			return std::unexpected(static_cast<error>(error_code));
		}

		if (is_focussed == focus::yes) {
			glfwFocusWindow(window_.get());
		}

		return {};
	}
} // namespace window
