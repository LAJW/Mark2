#include "../core_env.h"
#include <catch.hpp>

SCENARIO("core")
{
	GIVEN("undamaged core")
	{
		using namespace mark;
		core_env core_env;
		WHEN("we do nothing")
		{
			THEN("Core module position should be the same as the ship position")
			{
				let& core = core_env.core;
				REQUIRE(core.pos() == vd(0, 0));
				core_env.modular->pos(vd(13., 13.));
				REQUIRE(core.pos() == vd(13, 13));
			}
			THEN("Core module should not be detachable")
			{
				REQUIRE(core_env.core.detachable() == false);
			}
			THEN("Core module should be passive")
			{
				REQUIRE(core_env.core.passive() == true);
			}
		}
	}
}
