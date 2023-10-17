#ifndef BUGGY_VULKAN_ERROR_HPP
#define BUGGY_VULKAN_ERROR_HPP

#include <GLFW/glfw3.h>
#include <cstdint>
#include <expected>
#include <vulkan/vulkan.h>

namespace vulkan {
	enum class error : std::int32_t {
		// Vulkan errors
		not_ready = VK_NOT_READY,
		timeout = VK_TIMEOUT,
		event_set = VK_EVENT_SET,
		event_reset = VK_EVENT_RESET,
		incomplete = VK_INCOMPLETE,
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
		pipeline_compile_required = VK_PIPELINE_COMPILE_REQUIRED,
		surface_lost = VK_ERROR_SURFACE_LOST_KHR,
		native_window_in_use = VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
		suboptimal = VK_SUBOPTIMAL_KHR,
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
		thread_idle = VK_THREAD_IDLE_KHR,
		thread_done = VK_THREAD_DONE_KHR,
		operation_deferred = VK_OPERATION_DEFERRED_KHR,
		operation_not_deferred = VK_OPERATION_NOT_DEFERRED_KHR,
		compression_exhausted = VK_ERROR_COMPRESSION_EXHAUSTED_EXT,
		incompatible_shader_binary = VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT,

		// API errors
		no_suitable_devices = 1,
		file_not_found,
	};

	template<class T>
	using error_or = std::expected<T, error>;
} // namespace vulkan

#endif // BUGGY_VULKAN_ERROR_HPP
