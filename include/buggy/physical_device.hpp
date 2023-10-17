#ifndef BUGGY_PHYSICAL_DEVICE_HPP
#define BUGGY_PHYSICAL_DEVICE_HPP

#include "buggy/semver.hpp"
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace vulkan {
	struct physical_device {
		enum class type : std::uint8_t {
			other = VK_PHYSICAL_DEVICE_TYPE_OTHER,
			integrated_gpu = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
			discrete_gpu = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
			virtual_gpu = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
			cpu = VK_PHYSICAL_DEVICE_TYPE_CPU,
		};
		using enum type;

		enum class feature : std::uint8_t {
			robust_buffer_access,
			full_draw_index_uint32,
			image_cube_array,
			independent_blend,
			geometry_shader,
			tessellation_shader,
			sample_rate_shading,
			dual_src_blend,
			logic_op,
			multi_draw_indirect,
			draw_indirect_first_instance,
			depth_clamp,
			depth_bias_clamp,
			fill_mode_non_solid,
			depth_bounds,
			wide_lines,
			large_points,
			alpha_to_one,
			multi_viewport,
			sampler_anisotropy,
			texture_compression_et_c2,
			texture_compression_astc_ldr,
			texture_compression_bc,
			occlusion_query_precise,
			pipeline_statistics_query,
			vertex_pipeline_stores_and_atomics,
			fragment_stores_and_atomics,
			shader_tessellation_and_geometry_point_size,
			shader_image_gather_extended,
			shader_storage_image_extended_formats,
			shader_storage_image_multisample,
			shader_storage_image_read_without_format,
			shader_storage_image_write_without_format,
			shader_uniform_buffer_array_dynamic_indexing,
			shader_sampled_image_array_dynamic_indexing,
			shader_storage_buffer_array_dynamic_indexing,
			shader_storage_image_array_dynamic_indexing,
			shader_clip_distance,
			shader_cull_distance,
			shader_float64,
			shader_int64,
			shader_int16,
			shader_resource_residency,
			shader_resource_min_lod,
			sparse_binding,
			sparse_residency_buffer,
			sparse_residency_image2_d,
			sparse_residency_image3_d,
			sparse_residency2_samples,
			sparse_residency4_samples,
			sparse_residency8_samples,
			sparse_residency16_samples,
			sparse_residency_aliased,
			variable_multisample_rate,
			inherited_queries,
		};

		[[nodiscard]] auto api_version() const noexcept -> semver;
		[[nodiscard]] auto driver_version() const noexcept -> semver;
		[[nodiscard]] auto vendor_id() const noexcept -> std::uint32_t;
		[[nodiscard]] auto device_id() const noexcept -> std::uint32_t;
		[[nodiscard]] auto hardware_type() const noexcept -> type;
		[[nodiscard]] auto name() const noexcept -> std::string_view;
		[[nodiscard]] auto limits() const noexcept -> VkPhysicalDeviceLimits;
		[[nodiscard]] auto sparse_properties() const noexcept -> VkPhysicalDeviceSparseProperties;

		[[nodiscard]] auto has_feature(feature feature) const noexcept -> bool;
		[[nodiscard]] auto has_extension(std::string_view extension_name, semver vulkan_version) const noexcept -> bool;

		[[nodiscard]] auto memory_types() const noexcept -> std::span<VkMemoryType const>;
		[[nodiscard]] auto memory_heaps() const noexcept -> std::span<VkMemoryHeap const>;

		VkPhysicalDevice device;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceMemoryProperties memory_properties;
		std::vector<VkExtensionProperties> extensions;
	};
} // namespace vulkan

#endif // BUGGY_PHYSICAL_DEVICE_HPP
