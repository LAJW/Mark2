#pragma once

namespace mark {

/// Reference wrapper for wrapping mutable reference type arguments
/// Forces explicit calling of ref(value) before passing into the function, EVEN
/// IF said function already has a ref
/// Thanks to C++17, you can specify the function as follows:
///   void func(ref<std::string> value);
/// And cal it like so:
///   std::string my_string;
///   func(ref(my_string)); <- You don't have to specify class template
///                            arguments
template <typename T>
class ref final
{
public:
	static_assert(
		!std::is_const_v<T>,
		"ref is only for passing non-const references. Use regular "
		"reference for const references");

	constexpr explicit ref(T& value)
		: ref_(value)
	{}
	constexpr ref(ref<T>&& value)
		: ref_(*value)
	{}
	constexpr explicit const ref(const ref& other) = default;
	template <typename U>
	constexpr ref(ref<U>&& other)
		: ref_(*other)
	{}

	[[nodiscard]] constexpr T& operator*() const { return ref_; }
	[[nodiscard]] constexpr T* operator->() const { return &ref_; }

private:
	T& ref_;
};

template <typename T>
ref(ref<T>& value)->ref<T>;
} // namespace mark
