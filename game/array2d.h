#pragma once
#include <algorithm/range.h>
#include <algorithm/enumerate.h>
#include <stdafx.h>
#include <add_const_if.h>

namespace mark {

template <typename T, size_t size_x, size_t size_y>
class array2d final
{
private:
	using data_type = std::array<T, size_x * size_y>;
	unique_ptr<data_type> m_data;

	template <typename U>
	static auto at(U& self, const vector<size_t>& pos)
	{
		Expects(pos.x < size_x);
		Expects(pos.y < size_y);
		return std::ref((*self.m_data)[pos.x % size_y + pos.y * size_x]);
	}

public:
	static constexpr const bool is_2d_container = true;
	using value_type = T;
	using size_type = vector<size_t>;

	array2d()
		: m_data(std::make_unique<data_type>())
	{}
	auto operator[](const vector<size_t>& pos) -> T& { return at(*this, pos); }
	auto operator[](const vector<size_t>& pos) const -> const T&
	{
		return at(*this, pos).get();
	}
	auto data() const -> const data_type& { return *m_data; }
	auto size() const -> size_type { return { size_x, size_y }; }
	void fill(const T& t) { m_data->fill(t); }
};

/// 2D Array Enumerator use by calling `enumerate(instanceOfArray2d)`
/// Behaves just like a regular array enumerator, only the index is a 2D vector
template <typename T>
class enumerator<T, std::enable_if_t<T::is_2d_container>> final
{
private:
	template <typename value_type>
	class iterator_impl final
	{
		using iterator_t = typename range_t<typename T::size_type>::iterator;

	public:
		using iterator_category = std::bidirectional_iterator_tag;
		typedef value_type value_type;
		using reference = value_type&;
		using pointer = value_type*;
		using difference_type = void;

		iterator_impl(T& container, iterator_t it) noexcept
			: m_it(it)
			, m_container(container)
		{}
		auto& operator++() noexcept
		{
			++m_it;
			return *this;
		}
		auto& operator--() noexcept
		{
			--m_it;
			return *this;
		}
		auto operator*() const noexcept -> value_type
		{
			return { *m_it, m_container[*m_it] };
		}
		auto operator==(const iterator_impl& other) const noexcept
		{
			return &m_container == &other.m_container && m_it == other.m_it;
		}
		auto operator!=(const iterator_impl& other) const noexcept
		{
			return !(*this == other);
		}

	private:
		T& m_container;
		iterator_t m_it;
	};

public:
	using iterator = iterator_impl<std::pair<
		typename T::size_type,
		add_const_if_t<typename T::value_type, std::is_const_v<T>>&>>;

	using const_iterator = iterator_impl<
		std::pair<typename T::size_type, typename const T::value_type&>>;

	enumerator(T& container)
		: m_container(container)
		, m_range(m_container.size())
	{}

	auto begin() noexcept -> iterator
	{
		return { m_container, m_range.begin() };
	}
	auto begin() const noexcept { return this->cbegin(); }
	auto cbegin() const noexcept -> const_iterator
	{
		return { m_container, m_range.begin() };
	}
	auto end() noexcept -> iterator { return { m_container, m_range.end() }; }
	auto end() const noexcept { return this->cend(); }
	auto cend() const noexcept -> const_iterator
	{
		return { m_container, m_range.end() };
	}

private:
	T& m_container;
	range_t<typename T::size_type> m_range;
};

} // namespace mark
