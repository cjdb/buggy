#include "buggy/device.hpp"
#include "buggy/error.hpp"
#include "buggy/instance.hpp"
#include <algorithm>

using std::chrono::nanoseconds;

namespace vulkan {
	static auto has_queue(
	  VkPhysicalDevice const device,
	  surface const* const surface,
	  std::span<queue::property const> const properties) noexcept -> std::vector<std::pair<queue::property, std::uint32_t>>
	{
		auto result = std::vector<std::pair<queue::property, std::uint32_t>>();
		auto const exclusive = static_cast<std::size_t>(std::ranges::count(properties, true, &queue::property::is_exclusive));
		result.reserve(exclusive + static_cast<std::size_t>(exclusive < properties.size()));

		if (surface != nullptr) {
		}
	}

	auto device::create(
	  instance const& instance,
	  surface const* const surface,
	  selector_fn const selector,
	  std::span<queue::property const> const properties,
	  std::span<char const* const> const extensions,
	  VkAllocationCallbacks const* const allocator) noexcept -> error_or<device>
	{
		auto physical_devices = instance.physical_devices();
		auto family_index = std::uint32_t{0};
		auto physical_device = std::ranges::find_if(physical_devices, selector);

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

	error_or<void> device::wait_one(std::span<VkFence const> const fences, nanoseconds const timeout) noexcept
	{
		auto const result = vkWaitForFences(
		  device_.get(),
		  static_cast<std::uint32_t>(fences.size()),
		  fences.data(),
		  VK_FALSE,
		  static_cast<std::uint64_t>(timeout.count()));

		if (result != VK_SUCCESS) {
			return std::unexpected(result != VK_TIMEOUT ? static_cast<error>(result) : error::timeout);
		}

		return {};
	}

	error_or<void> device::wait_all(std::span<VkFence const> const fences, nanoseconds const timeout) noexcept
	{
		auto const result = vkWaitForFences(
		  device_.get(),
		  static_cast<std::uint32_t>(fences.size()),
		  fences.data(),
		  VK_TRUE,
		  static_cast<std::uint64_t>(timeout.count()));

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

	error_or<void> device::submit(queue& queue, std::span<queue::submission const> const submissions, fence& fence) noexcept
	{
		auto const result =
		  vkQueueSubmit(queue.get(), static_cast<std::uint32_t>(submissions.size()), submissions.data(), fence.get());
		if (result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return {};
	}

	device::device(VkDevice const device, VkAllocationCallbacks const* const allocator, physical_device const& physical_device) noexcept
	: device_(device, {vkDestroyDevice, allocator})
	, physical_device_(&physical_device)
	{}

	[[nodiscard]] auto device::get() const noexcept -> VkDevice
	{
		return device_.get();
	}

	[[nodiscard]] auto device::get_physical_device() const noexcept -> physical_device const&
	{
		return *physical_device_;
	}

	auto device::wait_till_idle() const noexcept -> error_or<void>
	{
		if (auto const result = vkDeviceWaitIdle(device_.get()); result != VK_SUCCESS) {
			return std::unexpected(static_cast<error>(result));
		}

		return {};
	}
} // namespace vulkan
