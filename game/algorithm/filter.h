#pragma once
#include <stdafx.h>

namespace mark {
template <typename in_t, typename pred_t>
auto filter(in_t&& in, pred_t pred)
	-> std::vector<typename std::remove_reference_t<in_t>::value_type>
{
	std::vector<typename std::remove_reference_t<in_t>::value_type> out;
	out.reserve(in.size());
	if constexpr (std::is_rvalue_reference_v<decltype(in)>) {
		std::copy_if(
			std::make_move_iterator(std::begin(in)),
			std::make_move_iterator(std::end(in)),
			std::inserter(out, out.end()),
			pred);
	} else {
		std::copy_if(
			std::begin(in), std::end(in), std::inserter(out, out.end()), pred);
	}
	return out;
}
} // namespace mark
