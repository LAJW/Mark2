#include "mostly_unchanged.h"

auto mark::mostly_unchanged(
	const std::string& left,
	const std::string& right,
	size_t limit) -> bool
{
	let begin =
		std::mismatch(left.begin(), left.end(), right.begin(), right.end());
	if (begin.first == left.end()) {
		return true;
	}
	let end =
		std::mismatch(left.rbegin(), left.rend(), right.rbegin(), right.rend());
	return (end.first.base() - begin.first) + (end.second.base() - begin.second)
		< gsl::narrow<int64_t>(limit);
}
