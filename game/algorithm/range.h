#pragma once
#include <type_traits>
#include <iterator>

namespace mark {

// Vector/Area range_t
template <typename T, typename = void>
class range_t
{
public:
	class iterator final
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = T;
		using reference = value_type&;
		using pointer = value_type*;
		using difference_type = void;

		iterator(const range_t<T>& area, T i)
			: m_area(area)
			, m_i(i)
		{}
		iterator& operator++() noexcept
		{
			m_i.x++;
			if (m_i.x == m_area.bottom_right.x) {
				m_i.x = m_area.top_left.x;
				m_i.y++;
			}
			return *this;
		}
		iterator& operator--() noexcept
		{
			if (m_i.x > m_area.top_left.x) {
				m_i.x--;
			} else {
				m_i.x = m_area.bottom_right.x - 1;
				m_i.y--;
			}
			return *this;
		}
		auto operator==(const iterator& other) const noexcept
		{
			return other.m_i == m_i;
		}
		auto operator!=(const iterator& other) const noexcept
		{
			return other.m_i != m_i;
		}
		auto operator*() const noexcept { return m_i; }

	private:
		const range_t<T>& m_area;
		T m_i = 0;
	};
	using const_iterator = iterator;
	range_t(const T& bottom_right)
		: range_t({ 0, 0 }, bottom_right)
	{}
	range_t(const T& top_left, const T& bottom_right)
		: top_left(top_left)
		, bottom_right(bottom_right)
	{}
	const_iterator begin() const noexcept { return iterator(*this, top_left); }
	const_iterator end() const noexcept
	{
		return iterator(*this, { top_left.x, bottom_right.y });
	}
	const T top_left;
	const T bottom_right;
};

// Integer range range_t
template <typename T>
class range_t<T, std::enable_if_t<std::is_arithmetic_v<T>>>
{
public:
	class iterator final
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = T;
		using reference = T&;
		using pointer = T*;
		using difference_type = void;

		iterator(T value)
			: m_value(value)
		{}
		auto& operator++() noexcept
		{
			m_value++;
			return *this;
		}
		auto& operator--() noexcept
		{
			m_value--;
			return *this;
		}
		auto operator*() const noexcept { return m_value; }
		auto operator==(const iterator& other) const noexcept
		{
			return m_value == other.m_value;
		}
		auto operator!=(const iterator& other) const noexcept
		{
			return m_value != other.m_value;
		}

	private:
		T m_value;
	};
	using const_iterator = iterator;
	range_t(T end)
		: m_begin(0)
		, m_end(end)
	{}
	range_t(T begin, T end)
		: m_begin(begin)
		, m_end(end)
	{}
	iterator begin() const noexcept { return m_begin; }
	iterator end() const noexcept { return m_end; }

private:
	iterator m_begin;
	iterator m_end;
};

template <typename T>
auto range(const T& min, const T& max)
{
	return range_t<T>(min, max);
}

template <typename T>
auto range(const T& max)
{
	return range_t<T>(max);
}

} // namespace mark
