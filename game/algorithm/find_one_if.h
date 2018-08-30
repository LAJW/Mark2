#pragma once

namespace mark {
template <typename container_type, typename pred_type>
auto find_one_if(container_type& container, pred_type pred)
	-> optional<decltype(*std::begin(container))>
{
	let begin = std::begin(container);
	let end = std::end(container);
	let it = std::find_if(begin, end, std::move(pred));
	if (it == end) {
		return {};
	} else {
		return *it;
	}
}
} // namespace mark
