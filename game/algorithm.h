#pragma once
#include "stdafx.h"
#include <iterator>
#include <list>

namespace mark {

// Vector/Area range_t
// TODO: Type static assert to check whether T is specialization of vector
template <typename T, typename = void>
class range_t {
public:
	class iterator
		: public std::iterator<std::bidirectional_iterator_tag, T, void> {
	public:
		iterator(const range_t<T>& area, T i)
			: m_area(area)
			, m_i(i)
		{
		}
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
			}
			else {
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
		: range_t({0, 0}, bottom_right)
	{
	}
	range_t(const T& top_left, const T& bottom_right)
		: top_left(top_left)
		, bottom_right(bottom_right)
	{
	}
	const_iterator begin() const noexcept { return iterator(*this, top_left); }
	const_iterator end() const noexcept
	{
		return iterator(*this, {top_left.x, bottom_right.y});
	}
	const T top_left;
	const T bottom_right;
};

// Container Enumerator
template <typename T>
class enumerator final {
private:
	template <typename value_t, typename iterator_t>
	class iterator_impl final : public std::iterator<
									std::forward_iterator_tag,
									std::pair<typename T::size_type, value_t&>,
									void> {
	public:
		iterator_impl(iterator_t it, typename T::size_type i) noexcept
			: m_it(it)
			, m_i(i)
		{
		}
		auto& operator++() noexcept
		{
			++m_it;
			++m_i;
			return *this;
		}
		auto operator*() noexcept -> std::pair<typename T::size_type, value_t&>
		{
			return {m_i, *m_it};
		}
		auto operator*() const noexcept
			-> std::pair<typename T::size_type, const value_t&>
		{
			return {m_i, *m_it};
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
	{
	}
	auto begin() noexcept -> iterator { return {m_container.begin(), 0}; }
	auto begin() const noexcept -> const_iterator
	{
		return {m_container.cbegin(), 0};
	}
	auto cbegin() const noexcept -> const_iterator
	{
		return {m_container.cbegin(), 0};
	}
	auto end() noexcept -> iterator
	{
		return {m_container.end(), m_container.size()};
	}
	auto end() const noexcept -> const_iterator
	{
		return {m_container.cend(), m_container.size()};
	}
	auto cend() const noexcept -> const_iterator
	{
		return {m_container.cend(), m_container.size()};
	}

private:
	T& m_container;
};

// Integer range range_t
template <typename T>
class range_t<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
public:
	class iterator
		: public std::iterator<std::bidirectional_iterator_tag, T, void> {
	public:
		iterator(T value)
			: m_value(value)
		{
		}
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
	{
	}
	range_t(T begin, T end)
		: m_begin(begin)
		, m_end(end)
	{
	}
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

template <typename iterator_t, typename pred_t>
auto has_if(iterator_t begin, iterator_t end, pred_t& pred)
{
	return std::find_if(begin, end, pred) != end;
}

template <typename container_t, typename pred_t>
auto has_if(container_t container, pred_t& pred)
{
	let begin = std::begin(container);
	let end = std::end(container);
	return std::find_if(begin, end, pred) != end;
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

// Diff two lists. Return list of iterators to elements to be removed from
// the old list and a list of elements to be added to the old list
template <typename old_container_t, typename new_container_t, typename equals_t>
auto diff(
	const old_container_t& old_list,
	const new_container_t& new_list,
	const equals_t& equals)
	-> std::pair<
		std::vector<typename old_container_t::const_iterator>,
		std::vector<std::pair<
			typename old_container_t::const_iterator,
			typename new_container_t::value_type>>>
{
	std::vector<typename old_container_t::const_iterator> removed;
	std::vector<std::pair<
		typename old_container_t::const_iterator,
		typename new_container_t::value_type>>
		added;
	auto old_it = old_list.begin();
	let old_end = old_list.end();
	auto new_it = new_list.begin();
	let new_end = new_list.end();
	while (old_it != old_end || new_it != new_end) {
		if (old_it == old_end) {
			added.push_back({old_end, *new_it});
			++new_it;
		}
		else if (new_it == new_end) {
			removed.push_back(old_it);
			++old_it;
		}
		else if (equals(*old_it, *new_it)) {
			++new_it;
			++old_it;
		}
		else {
			let found_new = std::find_if(new_it, new_end, [&](let& value) {
				return equals(*old_it, value);
			});
			if (found_new == new_end) {
				removed.push_back(old_it);
				++old_it;
			}
			else {
				for (auto it = new_it; it != found_new; ++it) {
					added.push_back({old_it, *it});
				}
				new_it = std::next(found_new);
				++old_it;
			}
		}
	}
	return {removed, added};
}

template <typename old_container_t, typename new_container_t>
auto diff(const old_container_t& old_list, const new_container_t& new_list)
{
	return diff(old_list, new_list, [](let& a, let& b) { return a == b; });
}

} // namespace mark