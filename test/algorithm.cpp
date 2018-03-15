#include "../game/stdafx.h"
#include "../game/algorithm.h"
#include <catch.hpp>

TEST_CASE("Enumerate container")
{
	const std::vector<int> container = {1, 2, 3, 4, 5};
	std::vector<std::pair<size_t, int>> out;
	for (let& pair : mark::enumerate(container)) {
		out.push_back({pair.first, pair.second});
	}
	for (size_t i = 0; i < container.size(); i++) {
		REQUIRE(i == out[i].first);
		REQUIRE(container[i] == out[i].second);
	}
}

TEST_CASE("Enumerate an area - implicit start")
{
	int i = 0;
	for (let& point : mark::range(mark::vector<int>(3, 2))) {
		REQUIRE(point == mark::vector<int>(i % 3, i / 3));
		++i;
	}
	REQUIRE(i == 3 * 2);
}

TEST_CASE("Enumerate an area")
{
	using namespace mark;
	int i = 0;
	for (let& point : range<vector<int>>({1, 1}, {4, 3})) {
		REQUIRE(point == vector<int>(i % 3 + 1, i / 3 + 1));
		++i;
	}
	REQUIRE(i == 3 * 2);
}

TEST_CASE("Diff two identical lists")
{
	std::list<int> old_list{1, 2, 5, 7};
	std::list<int> new_list{1, 2, 5, 7};
	let[removed, added] = mark::diff(old_list, new_list);
	REQUIRE(removed.empty());
	REQUIRE(added.empty());
}

TEST_CASE("Diff lists, last element missing")
{
	std::list<int> old_list{1, 2, 5, 7};
	std::list<int> new_list{1, 2, 5};
	let[removed, added] = mark::diff(old_list, new_list);
	REQUIRE(removed == decltype(removed){std::prev(old_list.end())});
	REQUIRE(added.empty());
}

TEST_CASE("Diff lists, additional element added to the end")
{
	std::list<int> old_list{1, 2, 5};
	std::list<int> new_list{1, 2, 5, 7};
	let[removed, added] = mark::diff(old_list, new_list);
	REQUIRE(removed.empty());
	REQUIRE(added == decltype(added){std::make_pair(old_list.end(), 7)});
}

TEST_CASE("Diff lists, middle element missing")
{
	std::list<int> old_list{1, 2, 5, 7};
	std::list<int> new_list{1, 5, 7};
	let[removed, added] = mark::diff(old_list, new_list);
	REQUIRE(removed == decltype(removed){std::next(old_list.begin())});
	REQUIRE(added.empty());
}

TEST_CASE("Diff lists, new middle element")
{
	std::list<int> old_list{1, 2, 7};
	std::list<int> new_list{1, 2, 5, 7};
	let[removed, added] = mark::diff(old_list, new_list);
	REQUIRE(removed.empty());
	REQUIRE(
		added == decltype(added){std::make_pair(std::prev(old_list.end()), 5)});
}