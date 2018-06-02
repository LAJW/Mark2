#pragma once
#include "stdafx.h"
#include <iterator>
#include <list>

namespace mark {

template <typename T, typename... Args>
auto make_shared(Args&&... args) -> not_null<shared_ptr<T>>
{
	return std::make_shared<T>(std::forward<Args>(args)...);
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
