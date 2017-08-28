#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "../game/resource_manager.h"
#include "../game/map.h"

// Map

TEST_CASE("should return nullptr or possibly crash") {
	mark::resource::manager_stub rm;
	const auto map = mark::map::make_square(rm);
	const auto maybe_result = map.collide({
		{ 0, 0 },
		{ 1000, 0 }
	});
	REQUIRE(maybe_result.has_value());
	const auto result = maybe_result.value();
	REQUIRE(result.x == Approx(8.5 * mark::map::tile_size));
	REQUIRE(result.y == Approx(0));
}
