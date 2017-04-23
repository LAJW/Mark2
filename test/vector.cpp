#include "catch.hpp"
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
	REQUIRE(std::round(mark::distance(45, { 0, 1 }) * 1000.0) == std::round(1 / sqrt(2) * 1000.0));
}

TEST_CASE("tan stress: 90 deg line, distance should be 0") {
	REQUIRE(std::round(mark::distance(90, { 0, 1 }) * 1000.0) == 0.0);
}

TEST_CASE("tan stress: 90 deg line, distance should be 1") {
	REQUIRE(std::round(mark::distance(90, { 1, 1 }) * 1000.0) == 1 * 1000.0);
}