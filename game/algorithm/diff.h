#pragma once
#include <stdafx.h>

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

