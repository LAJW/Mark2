#pragma once
#include <algorithm>

namespace mark {
template <typename container_t, typename pred_t>
auto find_if(const container_t& container, pred_t pred)
{
	return std::find_if(std::begin(container), std::end(container), pred);
}
} // namespace mark
