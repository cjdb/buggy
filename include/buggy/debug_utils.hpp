#ifndef BUGGY_DEBUG_UTILS_HPP
#define BUGGY_DEBUG_UTILS_HPP

#include "deleter.hpp"
#include "error.hpp"
#include <memory>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace vulkan {
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
		std::unique_ptr<VkDebugUtilsMessengerEXT_T, deleter<PFN_vkDestroyDebugUtilsMessengerEXT, VkInstance>> messenger_;

		debug_utils(VkDebugUtilsMessengerEXT, VkInstance, VkAllocationCallbacks const*) noexcept;

		static VKAPI_ATTR VkBool32 VKAPI_CALL log(
		  VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		  VkDebugUtilsMessageTypeFlagsEXT message_type,
		  VkDebugUtilsMessengerCallbackDataEXT const* message_data,
		  void* user_data) noexcept;
	};
} // namespace vulkan

#endif // BUGGY_DEBUG_UTILS_HPP
