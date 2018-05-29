﻿#pragma once
#include "stdafx.h"
#include <iterator>
#include <list>

namespace mark {

template <typename T, typename... Args>
auto make_shared(Args&&... args) -> not_null<shared_ptr<T>>
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

// Vector/Area range_t
// TODO: Type static assert to check whether T is specialization of vector
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

template <typename T>
auto enumerate(T& max)
{
	return enumerator<T>(max);
}

template <typename container_t, typename pred_t>
auto any_of(const container_t& container, pred_t pred)
{
	let begin = std::begin(container);
	let end = std::end(container);
	return std::any_of(begin, end, std::move(pred));
}

template <typename container_t, typename pred_t>
auto all_of(const container_t& container, pred_t pred)
{
	let begin = std::begin(container);
	let end = std::end(container);
	return std::all_of(begin, end, std::move(pred));
}

template <typename container_t, typename initial_t, typename pred_t>
initial_t
accumulate(const container_t& container, initial_t initial, pred_t pred)
{
	let begin = std::begin(container);
	let end = std::end(container);
	return std::accumulate(begin, end, std::move(initial), std::move(pred));
}

template <typename container_t, typename out_t, typename pred_t>
auto transform(const container_t& container, out_t out, pred_t pred)
{
	return std::transform(
		std::begin(container), std::end(container), out, std::move(pred));
}

template <typename container_t, typename pred_t>
auto count_if(const container_t& container, pred_t pred) noexcept
{
	let begin = std::begin(container);
	let end = std::end(container);
	return gsl::narrow<size_t>(std::count_if(begin, end, std::move(pred)));
}

// vector erase, don't preserve element order
template <typename vector_t, typename iterator_t>
auto drop(vector_t& vector, iterator_t it)
{
	auto owner = std::move(*it);
	std::swap(*it, vector.back());
	vector.pop_back();
	return std::move(owner);
}

template <typename old_container_t, typename new_container_t>
struct diff_result
{
	std::vector<typename old_container_t::const_iterator> removed;
	std::vector<std::pair<
		typename old_container_t::const_iterator,
		typename new_container_t::value_type>>
		added;
};

// Diff two lists. Return list of iterators to elements to be removed from
// the old list and a list of elements to be added to the old list
template <typename old_container_t, typename new_container_t, typename equals_t>
auto diff(
	const old_container_t& old_list,
	const new_container_t& new_list,
	equals_t equals) -> diff_result<old_container_t, new_container_t>
{
	diff_result<old_container_t, new_container_t> result;
	auto old_it = old_list.begin();
	let old_end = old_list.end();
	auto new_it = new_list.begin();
	let new_end = new_list.end();
	while (old_it != old_end || new_it != new_end) {
		if (old_it == old_end) {
			result.added.push_back({ old_end, *new_it });
			++new_it;
		} else if (new_it == new_end) {
			result.removed.push_back(old_it);
			++old_it;
		} else if (equals(*old_it, *new_it)) {
			++new_it;
			++old_it;
		} else {
			let found_new = std::find_if(new_it, new_end, [&](let& value) {
				return equals(*old_it, value);
			});
			if (found_new == new_end) {
				result.removed.push_back(old_it);
				++old_it;
			} else {
				for (auto it = new_it; it != found_new; ++it) {
					result.added.push_back({ old_it, *it });
				}
				new_it = std::next(found_new);
				++old_it;
			}
		}
	}
	return result;
}

template <typename old_container_t, typename new_container_t>
auto diff(const old_container_t& old_list, const new_container_t& new_list)
{
	return diff(old_list, new_list, [](let& a, let& b) { return a == b; });
}

/** Get minimum element and store its copy in an optional
 * @param begin: An iterator to the beginning of the range
 * @param end: An iterator to the end of the range
 * @param comp: Binary function comparing two elements
 * @return An optional storing a copy to the minimum element
 */
template <typename iterator_t, typename comparator_t>
auto min_element_v(iterator_t begin, iterator_t end, comparator_t comp)
	-> std::optional<std::remove_reference_t<decltype(*begin)>>
{
	let min_it = min_element(begin, end, comp);
	if (min_it != end) {
		return *min_it;
	}
	return {};
}

template <typename container_t, typename comparator_t>
auto min_element_v(const container_t& container, comparator_t comp)
{
	return mark::min_element_v(
		std::begin(container), std::end(container), comp);
}
} // namespace mark
