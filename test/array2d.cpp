#include <catch.hpp>
#include <array2d.h>

TEST_CASE("Create a 2D array of integers")
{
	using namespace mark;
	let constexpr array_size = 20;
	array2d<int, array_size, array_size> array;
	array[{ 0, 0 }] = 1;
	REQUIRE(array[{ 0, 0}] == 1);
	REQUIRE(array.size() == vector<size_t>(array_size, array_size));
	auto my_array = std::array<int, array_size * array_size>();
	REQUIRE(array.data().size() == array_size * array_size);
	REQUIRE(array.data()[0] == 1);
}