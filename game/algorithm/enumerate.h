#pragma once
#include <stdafx.h>

namespace mark {

// Container Enumerator
template <typename T, typename = void>
class enumerator final
{
private:
	template <typename value_t, typename iterator_t>
	class iterator_impl final
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = std::pair<typename T::size_type, value_t&>;
		using reference = value_type&;
		using pointer = value_type*;
		using difference_type = void;

		iterator_impl(iterator_t it, typename T::size_type i) noexcept
			: m_it(it)
			, m_i(i)
		{}
		auto& operator++() noexcept
		{
			++m_it;
			++m_i;
			return *this;
		}
		auto operator*() noexcept -> std::pair<typename T::size_type, value_t&>
		{
			return { m_i, *m_it };
		}
		auto operator*() const noexcept
			-> std::pair<typename T::size_type, const value_t&>
		{
			return { m_i, *m_it };
		}
		auto operator==(const iterator_impl& other) const noexcept
		{
			return m_it == other.m_it;
		}
		auto operator!=(const iterator_impl& other) const noexcept
		{
			return m_it != other.m_it;
		}

	private:
		iterator_t m_it;
		typename T::size_type m_i;
	};

public:
	using iterator =
		iterator_impl<decltype(*T().begin()), decltype(T().begin())>;
	using const_iterator =
		iterator_impl<decltype(*T().begin()), decltype(T().cbegin())>;

	enumerator(T& container)
		: m_container(container)
	{}
	auto begin() noexcept -> iterator { return { m_container.begin(), 0 }; }
	auto begin() const noexcept -> const_iterator
	{
		return { m_container.cbegin(), 0 };
	}
	auto cbegin() const noexcept -> const_iterator
	{
		return { m_container.cbegin(), 0 };
	}
	auto end() noexcept -> iterator
	{
		return { m_container.end(), m_container.size() };
	}
	auto end() const noexcept -> const_iterator
	{
		return { m_container.cend(), m_container.size() };
	}
	auto cend() const noexcept -> const_iterator
	{
		return { m_container.cend(), m_container.size() };
	}

private:
	T& m_container;
};

template <typename T>
auto enumerate(T& max)
{
	return enumerator<T>(max);
}

} // namespace mark
