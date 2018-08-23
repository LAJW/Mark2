#pragma once
#include <algorithm>

namespace mark {
template <typename container_t, typename pred_t>
[[nodiscard]] auto find_if(container_t& container, pred_t pred)
{
	// NOTE TO THE FUTURE SELF: Before you have a brilliant idea of turning
	// container_t into non-const, keep in mind that this function returns an
	// iterator. So container has to exist longer than this function. Putting it
	// as an argument of this makes little sense, as the iterator will be
	// invalidated, because the vector will no longer be there. You're welcome.
	return std::find_if(std::begin(container), std::end(container), pred);
}
} // namespace mark
