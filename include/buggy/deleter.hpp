#ifndef BUGGY_DELETER_HPP
#define BUGGY_DELETER_HPP

#include <utility>
#include <vulkan/vulkan.h>

namespace vulkan {
	template<class F, class Owner = void>
	class deleter {
	public:
		deleter(F f, Owner owner, VkAllocationCallbacks const* const allocator) noexcept
		: deleter_(std::move(f))
		, owner_(std::move(owner))
		, allocator_(allocator)
		{}

		template<class T>
		requires std::invocable<F, T, VkAllocationCallbacks const*>
		void operator()(T const t) const noexcept
		{
			if (t) {
				std::invoke(deleter_, owner_, t, allocator_);
			}
		}
	private:
		F deleter_;
		Owner owner_;
		VkAllocationCallbacks const* allocator_;
	};

	template<class F>
	class deleter<F, void> {
	public:
		deleter(F f, VkAllocationCallbacks const* const allocator) noexcept
		: deleter_(std::move(f))
		, allocator_(allocator)
		{}

		template<class T>
		requires std::invocable<F, T, VkAllocationCallbacks const*>
		void operator()(T const t) const noexcept
		{
			if (t) {
				std::invoke(deleter_, t, allocator_);
			}
		}
	private:
		F deleter_;
		VkAllocationCallbacks const* allocator_;
	};
} // namespace vulkan

#endif // BUGGY_DELETER_HPP
