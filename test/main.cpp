#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "../game/resource_manager.h"
#include "../game/world.h"

TEST_CASE("should return nullptr or possibly crash") {
	mark::resource::manager rm;
	mark::world world(rm);
	REQUIRE(world.camera() == mark::vector<double>(0, 0));
}