#include "../core_env.h"
#include <catch.hpp>
#include <exception.h>
#include <module/healing_turret.h>
#include <module/impl/healing_turret.h>

SCENARIO(
	"healing turret internals tested on a modular with two healing turrets")
{
	using namespace mark;
	core_env env;
	REQUIRE(success(env.modular->attach(
		{ -1, -3 },
		std::make_unique<mark::module::healing_turret>(
			env.rm, env.random, YAML::Node()))));
	let& left_turret = *env.modular->module_at({ -1, -3 });
	REQUIRE(success(env.modular->attach(
		{ -1, 1 },
		std::make_unique<mark::module::healing_turret>(
			env.rm, env.random, YAML::Node()))));
	let& right_turret = *env.modular->module_at({ -1, 1 });
	GIVEN("in_range function")
	{
		WHEN("called with range smaller than distance between these turrets")
		{
			let result = in_range(
				left_turret, right_turret, (4 * mark::module::size) - 1);
			THEN("It should return false") { REQUIRE(result == false); }
		}
		WHEN("called with range identical as distance between centers of these "
			 "turrets")
		{
			let result =
				in_range(left_turret, right_turret, 4 * mark::module::size);
			THEN("It should return true") { REQUIRE(result == true); }
		}
		WHEN("called with range larger than distance between these turrets")
		{
			let result =
				in_range(left_turret, right_turret, 4 * mark::module::size + 1);
			THEN("It should return true") { REQUIRE(result == true); }
		}
	}
	GIVEN("neighbor_at_pos_in_range function")
	{
		WHEN("We call neighbor_at_pos_in_range function pointing at self")
		{
			let result =
				neighbor_at_pos_in_range(left_turret, { -1, -3 }, 9000.);
			THEN("It should return an empty optional")
			{
				REQUIRE(!result.has_value());
			}
		}
		WHEN("We call neighbor_at_pos_in_range function pointing at the other "
			 "module, with a large range")
		{
			let result =
				neighbor_at_pos_in_range(left_turret, { -1, 1 }, 9000.);
			THEN("It should return a non-empty optional")
			{
				REQUIRE(result.has_value());
			}
			THEN("It should return an optional pointing at the other turret")
			{
				REQUIRE(result->equals(right_turret));
			}
		}
		WHEN("We call neighbor_at_pos_in_range function pointing at the other "
			 "module, with a range smaller than the distance between module "
			 "centers")
		{
			let result = neighbor_at_pos_in_range(
				left_turret, { -1, 1 }, 4 * mark::module::size - 1);
			THEN("It should return an empty optional")
			{
				REQUIRE(!result.has_value());
			}
		}
	}
	GIVEN("neighbors_in_radius function")
	{
		WHEN("we call it on the left healing turret and a large radius")
		{
			let modules = neighbors_in_radius(left_turret, 9000.);
			THEN("It should return a set containing all modules in the modular "
				 "except the left turret")
			{
				REQUIRE(modules.size() == 2);
				REQUIRE(modules.count(&right_turret) == 1);
				REQUIRE(modules.count(&env.core) == 1);
			}
		}
		WHEN("we call it on the left healing turret and a two-block radius")
		{
			let modules = neighbors_in_radius(left_turret, module::size * 2);
			THEN("It shoudl include the core"
				 "except the left turret")
			{
				REQUIRE(modules.size() == 1);
				REQUIRE(modules.count(&env.core) == 1);
			}
		}
	}
}