#include "../game/stdafx.h"
#include <catch.hpp>
#include "../game/vector.h"

TEST_CASE("Horizontal line, point 0, 0 - distance should be 0") {
	REQUIRE(mark::distance(0, { 0, 0 }) == 0);
}

TEST_CASE("Horizontal line, point 0, 1 - distance should be 1") {
	REQUIRE(mark::distance(0, { 0, 1 }) == 1);
}

TEST_CASE("Horizontal line, point 30, -2 - distance should be 2") {
	REQUIRE(mark::distance(0, { 30, -2 }) == 2);
}

TEST_CASE("45 deg line, distance should be 1/sqrt(2)") {
	let result = mark::distance(45, { 0, 1 });
	REQUIRE(result == Approx(1.0 / sqrt(2)));
}

TEST_CASE("tan stress: 90 deg line, distance should be 0") {
	let result = mark::distance(90, { 0, 1 });
	// Approx doesn't cut it here
	REQUIRE(result <= 0.00001);
	REQUIRE(result >= -0.00001);
}

TEST_CASE("tan stress: 90 deg line, distance should be 1") {
	REQUIRE(mark::distance(90, { 1, 1 }) == Approx(1));
}

// GET LINE

TEST_CASE("get_line same point, should result in a horizontal line") {
	using namespace mark;
	let result = get_line({ 3, 5 }, { 3, 5 });
	REQUIRE(std::holds_alternative<vector<double>>(result));
	let vec = std::get<vector<double>>(result);
	REQUIRE(vec.x == Approx(0));
	REQUIRE(vec.y == Approx(5));
}

TEST_CASE("get_line horizontal") {
	using namespace mark;
	let result = get_line({ 0, 0 }, { 1, 0 });
	REQUIRE(std::holds_alternative<vector<double>>(result));
	REQUIRE(std::get<vector<double>>(result) == mark::vector<double>(0, 0));
}

TEST_CASE("get_line vertical") {
	using namespace mark;
	let result = get_line({ 0, 0 }, { 0, 1 });
	REQUIRE(std::holds_alternative<double>(result));
	REQUIRE(std::get<double>(result) == 0);
}

TEST_CASE("get_line diagonal") {
	using namespace mark;
	let result = mark::get_line({ 3, 2 }, { 4, 3 });
	REQUIRE(std::holds_alternative<vector<double>>(result));
	let vec = std::get<vector<double>>(result);
	REQUIRE(vec.x == Approx(1.0));
	REQUIRE(vec.y == Approx(-1.0));
}

// Intersect

TEST_CASE("intersect horizontal and vertical on 0, 0") {
	using namespace mark;
	let result = intersect(vector<double>(), 0.);
	REQUIRE(result.has_value());
	REQUIRE(result->x == Approx(0));
	REQUIRE(result->y == Approx(0));
}

TEST_CASE("Intersect 2 different parallel lines") {
	using namespace mark;
	let result = intersect(vector<double>{ 3, 7 }, vector<double>{ 3, 8 });
	REQUIRE(!result.has_value());
}

TEST_CASE("Intersect the same line") {
	using namespace mark;
	let result = intersect(vector<double>{ 3, 7 }, vector<double>{ 3, 7 });
	REQUIRE(!result.has_value());
}

TEST_CASE("Crossing lines") {
	using namespace mark;
	let result = intersect(vector<double>{ 1, 3 }, vector<double>{ -1, 5 });
	REQUIRE(result.has_value());
	REQUIRE(result->x == Approx(1));
	REQUIRE(result->y == Approx(4));
}

// intersec(section, section)

TEST_CASE("Crossing sections, 45 degrees") {
	let result = mark::intersect(
		{ { -1, -1 }, { 1, 1 } },
		{ { -1, 1 }, { 1, -1 } }
	);
	REQUIRE(result.has_value());
	REQUIRE(result->x == Approx(0));
	REQUIRE(result->y == Approx(0));
}

TEST_CASE("Crossing sections at 90 degrees") {
	let result = mark::intersect(
		{ { 1, 1 }, { 1, -1 } },
		{ { -1, 0 }, { 1, 0 } }
	);
	REQUIRE(result.has_value());
	REQUIRE(result->x == Approx(1));
	REQUIRE(result->y == Approx(0));
}

TEST_CASE("Divergent sections should return [ NAN, NAN ]") {
	let result = mark::intersect(
		{ { -1, 1 }, { -1, -1 } },
		{ { 1, 1 }, { 0, 0 } }
	);
	REQUIRE(!result.has_value());
}

// Intersection(section, circle)

TEST_CASE("horizontal line and a circle at [ 0, 0 ]") {
	let result = mark::intersect(
		{ { -5, 0 }, { 5, 0 } },
		{ 0, 0 },
		1
	);
	REQUIRE(result.has_value());
	REQUIRE(result->x == Approx(-1));
	REQUIRE(result->y == Approx(0));
}

TEST_CASE("horizontal segment and a circle at [ 5, 3 ]") {
	let result = mark::intersect(
		{ { 5, 3 }, { 7, 3 } },
		{ 5, 3 },
		1
	);
	REQUIRE(result.has_value());
	REQUIRE(result->x == Approx(6));
	REQUIRE(result->y == Approx(3));
}

TEST_CASE("vertical line and a circle at [ 0, 0 ]") {
	let result = mark::intersect(
		{ { 0, -5 }, { 0, 5 } },
		{ 0, 0 },
		1
	);
	REQUIRE(result.has_value());
	REQUIRE(result->x == Approx(0));
	REQUIRE(result->y == Approx(-1));
}

TEST_CASE("45 deg line and a circle at [ 2, 2 ]") {
	let result = mark::intersect(
		{ { 0, 0 }, { 1, 1 } },
		{ 2, 2 },
		2
	);
	REQUIRE(result.has_value());
	REQUIRE(result->x == Approx(2.0 - std::sqrt(2)));
	REQUIRE(result->y == Approx(2.0 - std::sqrt(2)));
}

TEST_CASE("no collision (range check)") {
	let result = mark::intersect(
		{ { 8, 8 }, { 4.5, 4.5 } },
		{ 2, 2 },
		2
	);
	REQUIRE(!result.has_value());
}
