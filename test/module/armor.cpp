#include "../core_env.h"
#include <catch.hpp>
#include <module/armor.h>

// TODO: Move that somewhere accessible by all tests
SCENARIO("armor")
{
	GIVEN("a modular with a core and an armor module")
	{
		core_env env;
		REQUIRE(mark::success(env.modular->attach(
			{ 1, -1 }, make_item<mark::module::armor>(env.rm, env.random, [&] {
				auto _ = YAML::Node();
				_["armor"] = 20.f;
				return _;
			}()))));
		// We're relying on 0,0 being core
		REQUIRE(&env.core == env.modular->module_at({ 0, 0 }));
		WHEN("the core is damaged with physical damage")
		{
			mark::interface::damageable::damaged_set damaged, knocked;
			env.modular->module_at({ 0, 0 })->damage([&] {
				mark::interface::damageable::info _;
				_.damaged = damaged;
				_.knocked = knocked;
				_.random = env.random;
				_.physical = 25.f;
				_.team = 2;
				return _;
			}());
			THEN("Physical damage dealt should be reduced by the amount of "
				 "armor in the neighboring armor module")
			{
				REQUIRE(env.modular->module_at({ 0, 0 })->cur_health() == 95.f);
			}
		}
		WHEN("When physical damage dealt is lesser than armor, the minimum"
			 "amount of physical damage should be one")
		{
			mark::interface::damageable::damaged_set damaged, knocked;
			env.modular->module_at({ 0, 0 })->damage([&] {
				mark::interface::damageable::info _;
				_.damaged = damaged;
				_.knocked = knocked;
				_.random = env.random;
				_.physical = 10.f;
				_.team = 2;
				return _;
			}());
			THEN("Physical damage dealt should be reduced by the amount of "
				 "armor in the neighboring armor module")
			{
				REQUIRE(env.modular->module_at({ 0, 0 })->cur_health() == 99.f);
			}
		}
	}
}
