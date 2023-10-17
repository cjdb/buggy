#ifndef BUGGY_DEVICE_HPP
#define BUGGY_DEVICE_HPP

#include "deleter.hpp"
#include "error.hpp"
#include "physical_device.hpp"
#include <chrono>
#include <memory>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace vulkan {
	class fence;
	class instance;
	class surface;

	class queue {
	public:
		struct property {
			enum class type : std::int16_t {
				graphics = VK_QUEUE_GRAPHICS_BIT,
				compute = VK_QUEUE_COMPUTE_BIT,
				transfer = VK_QUEUE_TRANSFER_BIT,
				sparse_binding = VK_QUEUE_SPARSE_BINDING_BIT,
				protected_memory = VK_QUEUE_PROTECTED_BIT,
				video_decode = VK_QUEUE_VIDEO_DECODE_BIT_KHR,
#ifdef VK_ENABLE_BETA_EXTENSIONS
				video_encode = VK_QUEUE_VIDEO_ENCODE_BIT_KHR,
#endif
				optical_flow = VK_QUEUE_OPTICAL_FLOW_BIT_NV,
				present = optical_flow << 4
			};

			type property;
			bool is_exclusive;
		};

		using enum property::type;

		class submission;
	private:
		VkQueue queue_;
		property properties_;
	};

	class device {
	public:
		using selector_fn = auto (*)(physical_device const&) noexcept -> bool;

		[[nodiscard]] static auto create(
		  instance const& instance,
		  selector_fn selector,
		  std::span<queue::property const> properties,
		  std::span<char const* const> extensions = {},
		  VkAllocationCallbacks const* allocator = nullptr) noexcept -> error_or<device>;

		[[nodiscard]] static auto create(
		  instance const& instance,
		  surface const& surface,
		  selector_fn selector,
		  std::span<queue::property const> properties,
		  std::span<char const* const> extensions = {},
		  VkAllocationCallbacks const* allocator = nullptr) noexcept -> error_or<device>;

		[[nodiscard]] auto get() const noexcept -> VkDevice;
		[[nodiscard]] auto get_physical_device() const noexcept -> physical_device const&;

		[[nodiscard]] auto wait_one(
		  std::span<VkFence const> fences,
		  std::chrono::nanoseconds timeout = std::chrono::nanoseconds::min()) noexcept -> error_or<void>;

		[[nodiscard]] auto wait_all(
		  std::span<VkFence const> fences,
		  std::chrono::nanoseconds timeout = std::chrono::nanoseconds::min()) noexcept -> error_or<void>;

		[[nodiscard]] auto reset(std::span<VkFence const> fences) noexcept -> error_or<void>;

		[[nodiscard]] auto submit(queue& queue, std::span<queue::submission const> submissions, fence& f) noexcept
		  -> error_or<void>;

		[[nodiscard]] auto wait_till_idle() const noexcept -> error_or<void>;
	private:
		std::unique_ptr<VkDevice_T, deleter<PFN_vkDestroyDevice>> device_;
		physical_device const* physical_device_;
		std::vector<queue> queues_;

		explicit device(VkDevice, VkAllocationCallbacks const*, physical_device const&) noexcept;

		[[nodiscard]] auto create(
		  instance const&,
		  surface const*,
		  selector_fn,
		  std::span<queue::property const>,
		  std::span<char const* const>,
		  VkAllocationCallbacks const*) noexcept -> error_or<device>;
	};
} // namespace vulkan

#endif // BUGGY_DEVICE_HPP
