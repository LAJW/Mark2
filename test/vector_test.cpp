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
	const auto result = mark::distance(45, { 0, 1 });
	REQUIRE(result == Approx(1.0 / sqrt(2)));
}

TEST_CASE("tan stress: 90 deg line, distance should be 0") {
	const auto result = mark::distance(90, { 0, 1 });
	REQUIRE(result == Approx(0));
}

TEST_CASE("tan stress: 90 deg line, distance should be 1") {
	REQUIRE(mark::distance(90, { 1, 1 }) == Approx(1));
}

// GET LINE

TEST_CASE("get_line same point, should result in a horizontal line") {
	const auto result = mark::get_line({ 3, 5 }, { 3, 5 });
	REQUIRE(result.x == Approx(0));
	REQUIRE(result.y == Approx(5));
}

TEST_CASE("get_line horizontal") {
	REQUIRE(mark::get_line({ 0, 0 }, { 1, 0 }) == mark::vector<double>(0, 0));
}

TEST_CASE("get_line vertical") {
	const auto result = mark::get_line({ 0, 0 }, { 0, 1 });
	REQUIRE(result.x == 0);
	REQUIRE(std::isnan(result.y));
}

TEST_CASE("get_line diagonal") {
	const auto result = mark::get_line({ 3, 2 }, { 4, 3 });
	REQUIRE(result.y == Approx(-1.0));
	REQUIRE(result.x == Approx(1.0));
}

// Intersect

TEST_CASE("intersect horizontal and vertical on 0, 0") {
	const auto result = mark::intersect({ 0, 0 }, { 0, NAN });
	REQUIRE(result.x == Approx(0));
	REQUIRE(result.y == Approx(0));
}

TEST_CASE("Intersect 2 different parallel lines") {
	const auto result = mark::intersect({ 3, 7 }, { 3, 8 });
	REQUIRE(isnan(result.x));
	REQUIRE(isnan(result.y));
}

TEST_CASE("Intersect the same line") {
	const auto result = mark::intersect({ 3, 7 }, { 3, 7 });
	REQUIRE(isnan(result.x));
	REQUIRE(isnan(result.y));
}

TEST_CASE("Crossing lines") {
	const auto result = mark::intersect({ 1, 3 }, { -1, 5 });
	REQUIRE(result.x == Approx(1));
	REQUIRE(result.y == Approx(4));
}

// intersec(section, section)

TEST_CASE("Crossing sections, 45 degrees") {
	const auto result = mark::intersect(
		{ { -1, -1 }, { 1, 1 } },
		{ { -1, 1 }, { 1, -1 } }
	);
	REQUIRE(result.x == Approx(0));
	REQUIRE(result.y == Approx(0));
}

TEST_CASE("Crossing sections at 90 degrees") {
	const auto result = mark::intersect(
		{ { 1, 1 }, { 1, -1 } },
		{ { -1, 0 }, { 1, 0 } }
	);
	REQUIRE(result.x == Approx(1));
	REQUIRE(result.y == Approx(0));
}

TEST_CASE("Divergent sections should return [ NAN, NAN ]") {
	const auto result = mark::intersect(
		{ { -1, 1 }, { -1, -1 } },
		{ { 1, 1 }, { 0, 0 } }
	);
	REQUIRE(std::isnan(result.x));
	REQUIRE(std::isnan(result.y));
}

// Intersection(section, circle)

TEST_CASE("horizontal line and a circle at [ 0, 0 ]") {
	const auto result = mark::intersect(
		{ { -5, 0 }, { 5, 0 } },
		{ 0, 0 },
		1
	);
	REQUIRE(result.x == Approx(-1));
	REQUIRE(result.y == Approx(0));
}

TEST_CASE("horizontal segment and a circle at [ 5, 3 ]") {
	const auto result = mark::intersect(
		{ { 5, 3 }, { 7, 3 } },
		{ 5, 3 },
		1
	);
	REQUIRE(result.x == Approx(6));
	REQUIRE(result.y == Approx(3));
}

TEST_CASE("vertical line and a circle at [ 0, 0 ]") {
	const auto result = mark::intersect(
		{ { 0, -5 }, { 0, 5 } },
		{ 0, 0 },
		1
	);
	REQUIRE(result.x == Approx(0));
	REQUIRE(result.y == Approx(-1));
}

TEST_CASE("45 deg line and a circle at [ 2, 2 ]") {
	const auto result = mark::intersect(
		{ { 0, 0 }, { 1, 1 } },
		{ 2, 2 },
		2
	);
	REQUIRE(result.x == Approx(2.0 - std::sqrt(2)));
	REQUIRE(result.y == Approx(2.0 - std::sqrt(2)));
}

TEST_CASE("no collision (range check)") {
	const auto result = mark::intersect(
		{ { 8, 8 }, { 4.5, 4.5 } },
		{ 2, 2 },
		2
	);
	REQUIRE(std::isnan(result.x));
	REQUIRE(std::isnan(result.y));
}
