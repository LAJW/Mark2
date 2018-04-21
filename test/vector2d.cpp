#include <vector2d.h>
#include <catch.hpp>

TEST_CASE("Create a 2D vector of integers")
{
	using namespace mark;
	const constexpr size_t vector_size = 20;
	vector2d<int> vector({ vector_size, vector_size });
	vector[{ 0, 0 }] = 1;
	REQUIRE(vector[{ 0, 0 }] == 1);
	REQUIRE(vector.size() == mark::vector<size_t>(vector_size, vector_size));
	REQUIRE(vector.data().size() == vector_size * vector_size);
	REQUIRE(vector.data()[0] == 1);
}
