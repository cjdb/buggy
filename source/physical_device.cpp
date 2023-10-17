#include "buggy/physical_device.hpp"
#include "buggy/semver.hpp"
#include <algorithm>
#include <span>
#include <string_view>

namespace vulkan {
	auto physical_device::api_version() const noexcept -> semver
	{
		return semver(properties.apiVersion);
	}

	auto physical_device::driver_version() const noexcept -> semver
	{
		return semver(properties.driverVersion);
	}

	auto physical_device::vendor_id() const noexcept -> std::uint32_t
	{
		return properties.vendorID;
	}

	auto physical_device::device_id() const noexcept -> std::uint32_t
	{
		return properties.deviceID;
	}

	auto physical_device::hardware_type() const noexcept -> type
	{
		return static_cast<type>(properties.deviceType);
	}

	auto physical_device::name() const noexcept -> std::string_view
	{
		return properties.deviceName;
	}

	auto physical_device::limits() const noexcept -> VkPhysicalDeviceLimits
	{
		return properties.limits;
	}

	auto physical_device::sparse_properties() const noexcept -> VkPhysicalDeviceSparseProperties
	{
		return properties.sparseProperties;
	}

	auto physical_device::has_feature(feature const f) const noexcept -> bool
	{
		switch (f) {
		case feature::robust_buffer_access:
			return static_cast<bool>(features.robustBufferAccess);
		case feature::full_draw_index_uint32:
			return static_cast<bool>(features.fullDrawIndexUint32);
		case feature::image_cube_array:
			return static_cast<bool>(features.imageCubeArray);
		case feature::independent_blend:
			return static_cast<bool>(features.independentBlend);
		case feature::geometry_shader:
			return static_cast<bool>(features.geometryShader);
		case feature::tessellation_shader:
			return static_cast<bool>(features.tessellationShader);
		case feature::sample_rate_shading:
			return static_cast<bool>(features.sampleRateShading);
		case feature::dual_src_blend:
			return static_cast<bool>(features.dualSrcBlend);
		case feature::logic_op:
			return static_cast<bool>(features.logicOp);
		case feature::multi_draw_indirect:
			return static_cast<bool>(features.multiDrawIndirect);
		case feature::draw_indirect_first_instance:
			return static_cast<bool>(features.drawIndirectFirstInstance);
		case feature::depth_clamp:
			return static_cast<bool>(features.depthClamp);
		case feature::depth_bias_clamp:
			return static_cast<bool>(features.depthBiasClamp);
		case feature::fill_mode_non_solid:
			return static_cast<bool>(features.fillModeNonSolid);
		case feature::depth_bounds:
			return static_cast<bool>(features.depthBounds);
		case feature::wide_lines:
			return static_cast<bool>(features.wideLines);
		case feature::large_points:
			return static_cast<bool>(features.largePoints);
		case feature::alpha_to_one:
			return static_cast<bool>(features.alphaToOne);
		case feature::multi_viewport:
			return static_cast<bool>(features.multiViewport);
		case feature::sampler_anisotropy:
			return static_cast<bool>(features.samplerAnisotropy);
		case feature::texture_compression_et_c2:
			return static_cast<bool>(features.textureCompressionETC2);
		case feature::texture_compression_astc_ldr:
			return static_cast<bool>(features.textureCompressionASTC_LDR);
		case feature::texture_compression_bc:
			return static_cast<bool>(features.textureCompressionBC);
		case feature::occlusion_query_precise:
			return static_cast<bool>(features.occlusionQueryPrecise);
		case feature::pipeline_statistics_query:
			return static_cast<bool>(features.pipelineStatisticsQuery);
		case feature::vertex_pipeline_stores_and_atomics:
			return static_cast<bool>(features.vertexPipelineStoresAndAtomics);
		case feature::fragment_stores_and_atomics:
			return static_cast<bool>(features.fragmentStoresAndAtomics);
		case feature::shader_tessellation_and_geometry_point_size:
			return static_cast<bool>(features.shaderTessellationAndGeometryPointSize);
		case feature::shader_image_gather_extended:
			return static_cast<bool>(features.shaderImageGatherExtended);
		case feature::shader_storage_image_extended_formats:
			return static_cast<bool>(features.shaderStorageImageExtendedFormats);
		case feature::shader_storage_image_multisample:
			return static_cast<bool>(features.shaderStorageImageMultisample);
		case feature::shader_storage_image_read_without_format:
			return static_cast<bool>(features.shaderStorageImageReadWithoutFormat);
		case feature::shader_storage_image_write_without_format:
			return static_cast<bool>(features.shaderStorageImageWriteWithoutFormat);
		case feature::shader_uniform_buffer_array_dynamic_indexing:
			return static_cast<bool>(features.shaderUniformBufferArrayDynamicIndexing);
		case feature::shader_sampled_image_array_dynamic_indexing:
			return static_cast<bool>(features.shaderSampledImageArrayDynamicIndexing);
		case feature::shader_storage_buffer_array_dynamic_indexing:
			return static_cast<bool>(features.shaderStorageBufferArrayDynamicIndexing);
		case feature::shader_storage_image_array_dynamic_indexing:
			return static_cast<bool>(features.shaderStorageImageArrayDynamicIndexing);
		case feature::shader_clip_distance:
			return static_cast<bool>(features.shaderClipDistance);
		case feature::shader_cull_distance:
			return static_cast<bool>(features.shaderCullDistance);
		case feature::shader_float64:
			return static_cast<bool>(features.shaderFloat64);
		case feature::shader_int64:
			return static_cast<bool>(features.shaderInt64);
		case feature::shader_int16:
			return static_cast<bool>(features.shaderInt16);
		case feature::shader_resource_residency:
			return static_cast<bool>(features.shaderResourceResidency);
		case feature::shader_resource_min_lod:
			return static_cast<bool>(features.shaderResourceMinLod);
		case feature::sparse_binding:
			return static_cast<bool>(features.sparseBinding);
		case feature::sparse_residency_buffer:
			return static_cast<bool>(features.sparseResidencyBuffer);
		case feature::sparse_residency_image2_d:
			return static_cast<bool>(features.sparseResidencyImage2D);
		case feature::sparse_residency_image3_d:
			return static_cast<bool>(features.sparseResidencyImage3D);
		case feature::sparse_residency2_samples:
			return static_cast<bool>(features.sparseResidency2Samples);
		case feature::sparse_residency4_samples:
			return static_cast<bool>(features.sparseResidency4Samples);
		case feature::sparse_residency8_samples:
			return static_cast<bool>(features.sparseResidency8Samples);
		case feature::sparse_residency16_samples:
			return static_cast<bool>(features.sparseResidency16Samples);
		case feature::sparse_residency_aliased:
			return static_cast<bool>(features.sparseResidencyAliased);
		case feature::variable_multisample_rate:
			return static_cast<bool>(features.variableMultisampleRate);
		case feature::inherited_queries:
			return static_cast<bool>(features.inheritedQueries);
		}
	}

	auto physical_device::has_extension(std::string_view extension_name, semver vulkan_version) const noexcept -> bool
	{
		auto const result = std::ranges::find(extensions, extension_name, &VkExtensionProperties::extensionName);
		return result != extensions.end() and vulkan_version >= semver(result->specVersion);
	}

	auto physical_device::memory_types() const noexcept -> std::span<VkMemoryType const>
	{
		return {memory_properties.memoryTypes, memory_properties.memoryTypeCount};
	}

	auto physical_device::memory_heaps() const noexcept -> std::span<VkMemoryHeap const>
	{
		return {memory_properties.memoryHeaps, memory_properties.memoryHeapCount};
	}
} // namespace vulkan
