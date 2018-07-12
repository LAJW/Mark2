#pragma once
#include <stdafx.h>

namespace mark {
template <typename container_type, typename output_type, typename sum_type>
auto accumulate(
	const container_type& container,
	output_type first,
	sum_type sum)
{
	return std::accumulate(
		std::begin(container),
		std::end(container),
		std::move(first),
		std::move(sum));
}
} // namespace mark