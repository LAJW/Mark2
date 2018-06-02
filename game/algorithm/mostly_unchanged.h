#pragma once;
#include <stdafx.h>

namespace mark {
// Returns true if there's only a short difference between left and the right
// string
auto mostly_unchanged(
	const std::string& left,
	const std::string& right,
	size_t limit = 5) -> bool;
} // namespace mark
