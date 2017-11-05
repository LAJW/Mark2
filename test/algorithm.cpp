#include "../game/stdafx.h"
#include <catch.hpp>
#include "../game/algorithm.h"

TEST_CASE("Enumerate container")
{
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

TEST_CASE("Enumerate an area - implicit start")
{
	int i = 0;
	for (const auto& point : mark::range(mark::vector<int>(3, 2))) {
		REQUIRE(point == mark::vector<int>(i % 3, i / 3));
		++i;
	}
	REQUIRE(i == 3 * 2);
}

TEST_CASE("Enumerate an area")
{
	using namespace mark;
	int i = 0;
	for (const auto& point : range<vector<int>>({ 1, 1 }, { 4, 3 })) {
		REQUIRE(point == vector<int>(i % 3 + 1, i / 3 + 1));
		++i;
	}
	REQUIRE(i == 3 * 2);
}
