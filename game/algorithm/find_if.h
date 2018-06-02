#pragma once
#include <algorithm>

namespace mark {
template <typename container_t, typename pred_t>
auto find_if(container_t& container, pred_t pred)
{
	return std::find(std::begin(container), std::end(container), pred);
}
} // namespace mark
