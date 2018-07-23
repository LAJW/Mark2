#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <map.h>
#include <resource_manager.h>
#include "core_env.h"
#include <module/battery.h>

// Map

TEST_CASE("Map collide with a horizontal ray")
{
	mark::resource::manager_stub rm;
	mark::random_stub random;
	let map = mark::map::make_square(rm, random);
	let maybe_result = map.collide({ { 0, 0 }, { 1000, 0 } });
	REQUIRE(maybe_result.has_value());
	let result = maybe_result->pos;
	REQUIRE(result.x == Approx(8.5 * mark::map::tile_size));
	REQUIRE(result.y == Approx(0));
}
