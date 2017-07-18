#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "../game/resource_manager.h"
#include "../game/map.h"

// Map

TEST_CASE("should return nullptr or possibly crash") {
	mark::resource::manager_stub rm;
	const auto map = mark::map::make_square(rm);
	const auto result = map.collide({
		{ 0, 0 },
		{ 1000, 0 }
	});
	REQUIRE(result.x == Approx(8 * 32.0));
	REQUIRE(result.y == Approx(0));
}
