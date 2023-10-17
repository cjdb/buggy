#ifndef BUGGY_INSTANCE_HPP
#define BUGGY_INSTANCE_HPP

#include "deleter.hpp"
#include "error.hpp"
#include "physical_device.hpp"
#include <memory>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace vulkan {
	class instance {
	public:
		[[nodiscard]] auto create(
		  std::string_view application_name,
		  std::int32_t application_version,
		  std::string_view engine_name,
		  std::int32_t engine_version,
		  std::int32_t api_version,
		  std::span<char const* const> layers,
		  std::span<char const* const> extensions,
		  VkAllocationCallbacks const* allocator = nullptr) noexcept -> error_or<instance>;

		[[nodiscard]] std::span<physical_device const> physical_devices() const noexcept;
		[[nodiscard]] VkInstance get() const noexcept;
	private:
		std::unique_ptr<VkInstance_T, deleter<PFN_vkDestroyInstance>> insance_;
		std::vector<physical_device> physical_devices_;
	};
} // namespace vulkan

#endif // BUGGY_INSTANCE_HPP
