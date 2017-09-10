#include "../game/stdafx.h"
#include <catch.hpp>
#include "../game/algorithm.h"

TEST_CASE("Enumerate container") {
	const std::vector<int> container = { 1, 2, 3, 4, 5 };
	std::vector<std::pair<size_t, int>> out;
	for (const auto& pair : mark::enumerate(container)) {
		out.push_back({ pair.first, pair.second });
	}
	for (size_t i = 0; i < container.size(); i++) {
		REQUIRE(i == out[i].first);
		REQUIRE(container[i] == out[i].second);
	}
}
