#pragma once
#include <stdafx.h>

namespace mark {
template <typename T>
auto has_one(const T& container, const typename T::value_type& value)
{
	let begin = std::begin(container);
	let end = std::end(container);
	let it = std::find(begin, end, value);
	return it != end;
}
}