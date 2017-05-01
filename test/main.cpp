#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "../game/resource_manager.h"
#include "../game/map.h"

// Map

TEST_CASE("should return nullptr or possibly crash") {
	mark::resource::manager rm;
	const auto map = mark::map::make_square(rm);
}
