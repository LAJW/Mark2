#pragma once

namespace mark {

/// Reference wrapper for wrapping mutable reference type arguments
/// Forces explicit calling of ref(value) before passing into the function, EVEN
/// IF said function already has a ref
/// Thanks to C++17, you can specify the function as follows:
///   void func(ref<std::string> value);
/// And cal it:
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

	explicit ref(T& value)
		: m_ref(value)
	{}
	ref(T&& value)
		: m_ref(value)
	{}
	explicit const ref(const ref& other) = default;
	template <typename U>
	ref(const ref<U>& other)
		: m_ref(*other)
	{}

	T& operator*() const { return m_ref; }
	T* operator->() const { return &m_ref; }

private:
	T& m_ref;
};

template <typename T>
ref(ref<T>& value)->ref<T>;
} // namespace mark
