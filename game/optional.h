#pragma once
#include <gsl/gsl>
#include <optional>
#include <type_traits>

namespace mark {

template <typename T>
class optional_ref
{
	// Friending required for the aliasing constructor
	template<typename U>
	friend class optional_ref;
public:

	constexpr optional_ref() = default;
	constexpr optional_ref(const optional_ref&) = default;
	constexpr optional_ref(T& ref)
		: m_ptr(&ref)
	{}
	template <typename U>
	constexpr optional_ref(const optional_ref<U>& ref)
		: m_ptr(ref.m_ptr)
	{}
	constexpr T& operator*() const noexcept
	{
		Expects(m_ptr);
		return *m_ptr;
	}
	constexpr T* operator->() const noexcept
	{
		Expects(m_ptr);
		return m_ptr;
	}
	constexpr optional_ref<T> operator=(T& ref)
	{
		m_ptr = &ref;
		return *this;
	}
	constexpr operator bool() const { return m_ptr != nullptr; }
	constexpr T& value()
	{
		if (m_ptr) {
			return *m_ptr;
		}
		throw std::bad_optional_access();
	}
	constexpr bool has_value() const { return m_ptr != nullptr; }
	constexpr void reset() noexcept { m_ptr = nullptr; }

private:
	T* m_ptr = nullptr;
};

template <typename T, typename = void>
class optional final : public std::optional<T>
{
public:
	using std::optional<T>::optional;
	using std::optional<T>::operator=;
};

template <typename T>
class optional<T, std::enable_if_t<std::is_lvalue_reference_v<T>>>
	: public optional_ref<std::remove_reference_t<T>>
{
public:
	constexpr optional() = default;
	template <typename U>
	constexpr optional(const optional<U&>& ref)
		: optional_ref(static_cast<const optional_ref<U>>(ref))
	{}
	using optional_ref<std::remove_reference_t<T>>::optional_ref;
	using optional_ref<std::remove_reference_t<T>>::operator=;
};

} // namespace mark
