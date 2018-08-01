#pragma once
#include <algorithm>

namespace mark {
// For-each variant running the function N times, ignoring each element
template<typename T>
void for_each(const T& container, std::function<void()> func) {
	for (const auto& i : container) {
		(void)i;
		func();
	}
}
}
