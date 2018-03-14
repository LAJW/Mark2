#define CATCH_CONFIG_MAIN
#include "../game/map.h"
#include "../game/resource_manager.h"
#include <catch.hpp>

// Map

TEST_CASE("should return nullptr or possibly crash")
{
	mark::resource::manager_stub rm;
	let map = mark::map::make_square(rm);
	let maybe_result = map.collide({{0, 0}, {1000, 0}});
	REQUIRE(maybe_result.has_value());
	let result = maybe_result.value();
	REQUIRE(result.x == Approx(8.5 * mark::map::tile_size));
	REQUIRE(result.y == Approx(0));
}
