#ifndef BUGGY_SEMVER_HPP
#define BUGGY_SEMVER_HPP

#include <compare>
#include <cstdint>
#include <vector>

namespace vulkan {
	class semver {
	public:
		constexpr semver(std::int8_t const major, std::int8_t const minor, std::int8_t const patch) noexcept
		: major_(major)
		, minor_(minor)
		, patch_(patch)
		{}

		constexpr explicit semver(std::uint32_t const version) noexcept
		: major_(static_cast<std::int16_t>(version >> 22U))
		, minor_(static_cast<std::int16_t>((version & (~0U >> 10U)) >> 12U))
		, patch_(static_cast<std::int16_t>(version & (~0U >> 20U)))
		{}

		[[nodiscard]] constexpr auto major() const noexcept -> std::int16_t
		{
			return major_;
		}

		[[nodiscard]] constexpr auto minor() const noexcept -> std::int16_t
		{
			return minor_;
		}

		[[nodiscard]] constexpr auto patch() const noexcept -> std::int16_t
		{
			return patch_;
		}

		friend auto operator<=>(semver, semver) noexcept = default;
	private:
		std::int16_t major_;
		std::int16_t minor_;
		std::int16_t patch_;
	};
} // namespace vulkan

#endif // BUGGY_SEMVER_HPP
