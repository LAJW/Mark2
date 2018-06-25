#include <catch.hpp>
#include "../core_env.h"

TEST_CASE("Core module pos")
{
	using namespace mark;
	core_env core_env;
	let& core = core_env.core;
	REQUIRE(core.pos() == vd(0, 0));
	core_env.modular->pos(vd(13., 13.));
	REQUIRE(core.pos() == vd(13, 13));
}

TEST_CASE("Core module should not be detachable")
{
	using namespace mark;
	core_env core_env;
	REQUIRE(core_env.core.detachable() == false);
}

TEST_CASE("Core module should be passive")
{
	using namespace mark;
	core_env core_env;
	REQUIRE(core_env.core.passive() == true);
}
