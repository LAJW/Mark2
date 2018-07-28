#include <catch.hpp>
#include <exception.h>
#include <item/shard.h>
#include <module/core.h>
#include <module/cargo.h>
#include <module/turret.h>
#include <random.h>
#include <resource_manager.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>
#include <world_stack.h>

using namespace mark;

TEST_CASE("Create an empty modular")
{
	resource::manager_stub rm;
	random_stub random;
	world world(rm, random);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = world;
		_.pos = { 1, 2 };
		_.rotation = 3.f;
		return _;
	}());
	REQUIRE(modular.pos() == vector<double>(1, 2));
	REQUIRE(modular.rotation() == 3.f);
}

TEST_CASE("Create modular with a core")
{
	resource::manager_stub rm;
	mark::random_stub random;
	world world(rm, random);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = world;
		_.pos = { 1, 2 };
		_.rotation = 3.f;
		return _;
	}());
	std::unique_ptr<interface::item> core =
		std::make_unique<module::core>(rm, random, YAML::Node());
	let& core_ref = dynamic_cast<module::core&>(*core);
	REQUIRE(modular.attach({ -1, -1 }, move(core)) == error::code::success);
	REQUIRE(core == nullptr);
	REQUIRE(core_ref.pos() == vector<double>(1, 2));
	REQUIRE(core_ref.grid_pos() == vector<int>(-1, -1));
	REQUIRE(modular.at({ -1, -1 }) == &core_ref);
	REQUIRE(modular.at({ 0, -1 }) == &core_ref);
	REQUIRE(modular.at({ -1, 0 }) == &core_ref);
	REQUIRE(modular.at({ 0, 0 }) == &core_ref);
	REQUIRE(modular.at({ 1, -1 }) == nullptr);
}

TEST_CASE("Try creating modular with two cores")
{
	resource::manager_stub rm;
	random_stub random;
	world world(rm, random);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = world;
		return _;
	}());
	std::unique_ptr<interface::item> core1 =
		std::make_unique<module::core>(rm, random, YAML::Node());
	std::unique_ptr<interface::item> core2 =
		std::make_unique<module::core>(rm, random, YAML::Node());
	REQUIRE(error::code::success == modular.attach({ -1, -1 }, move(core1)));
	REQUIRE(error::code::success != modular.attach({ 1, -1 }, move(core2)));
}

TEST_CASE("Create modular with a turret to the right")
{
	resource::manager_stub rm;
	random_stub random;
	world world(rm, random);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = world;
		return _;
	}());
	std::unique_ptr<interface::item> core =
		std::make_unique<module::core>(rm, random, YAML::Node());
	REQUIRE(error::code::success == modular.attach({ -1, -1 }, move(core)));

	std::unique_ptr<interface::item> turret =
		std::make_unique<module::turret>(rm, random, YAML::Node());
	let& turret_ref = dynamic_cast<module::turret&>(*turret);
	REQUIRE(error::code::success == modular.attach({ 1, -1 }, move(turret)));
	REQUIRE(turret_ref.grid_pos() == vector<int>(1, -1));
	REQUIRE(turret_ref.pos().x == Approx(module::size * 2));
	REQUIRE(turret_ref.pos().y == Approx(0));
}

TEST_CASE("Attach turret in all possible positions")
{
	resource::manager_stub rm;
	random_stub random;
	world world(rm, random);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = world;
		return _;
	}());
	std::unique_ptr<interface::item> core =
		std::make_unique<module::core>(rm, random, YAML::Node());
	REQUIRE(error::code::success == modular.attach({ -1, -1 }, move(core)));

	std::unique_ptr<interface::item> turret =
		std::make_unique<module::turret>(rm, random, YAML::Node());
	let& turret_ref = *turret;

	// Right
	REQUIRE(error::code::success == modular.attach({ 1, -1 }, move(turret)));
	turret = modular.detach({ 1, -1 });

	REQUIRE(error::code::success == modular.attach({ 1, -2 }, move(turret)));
	turret = modular.detach({ 1, -2 });

	REQUIRE(error::code::success == modular.attach({ 1, 0 }, move(turret)));
	turret = modular.detach({ 1, 0 });

	// Left
	REQUIRE(error::code::success == modular.attach({ -3, -1 }, move(turret)));
	turret = modular.detach({ -3, -1 });

	REQUIRE(error::code::success == modular.attach({ -3, -2 }, move(turret)));
	turret = modular.detach({ -3, -2 });

	REQUIRE(error::code::success == modular.attach({ -3, 0 }, move(turret)));
	turret = modular.detach({ -3, 0 });

	// Top
	REQUIRE(error::code::success == modular.attach({ -1, 1 }, move(turret)));
	turret = modular.detach({ -1, 1 });

	REQUIRE(error::code::success == modular.attach({ -2, 1 }, move(turret)));
	turret = modular.detach({ -2, 1 });

	REQUIRE(error::code::success == modular.attach({ 0, 1 }, move(turret)));
	turret = modular.detach({ 0, 1 });

	// Left
	REQUIRE(error::code::success == modular.attach({ -1, -3 }, move(turret)));
	turret = modular.detach({ -1, -3 });

	REQUIRE(error::code::success == modular.attach({ -2, -3 }, move(turret)));
	turret = modular.detach({ -2, -3 });

	REQUIRE(error::code::success == modular.attach({ 0, -3 }, move(turret)));
	turret = modular.detach({ 0, -3 });
}

SCENARIO("modular")
{
	GIVEN("An empty modular with core in the center")
	{
		resource::manager_stub rm;
		random_stub random;
		world world(rm, random);
		auto modular = std::make_shared<unit::modular>([&] {
			unit::modular::info _;
			_.world = world;
			return _;
		}());
		REQUIRE(success(modular->attach(
			{ -1, -1 },
			std::make_unique<module::core>(rm, random, YAML::Node()))));
		world.attach(modular);
		WHEN("We do nothing")
		{
			THEN("3, -1 should be unoccupied")
			{
				REQUIRE(modular->at({ 3, -1 }) == nullptr);
			}
		}
		WHEN("We try to attach a module with zero health")
		{
			std::unique_ptr<interface::item> turret =
				std::make_unique<module::turret>(rm, random, [&] {
					YAML::Node node;
					node["cur_health"] = 0.f;
					return node;
				}());
			let error_code = modular->attach({ 3, -1 }, move(turret));
			THEN("Attach should return an error")
			{
				REQUIRE(failure(error_code));
			}
			THEN("Module shouldn't be reserved in modular's grid")
			{
				REQUIRE(modular->at({ 3, -1 }) == nullptr);
				REQUIRE(modular->module_at({ 3, -1 }) == nullptr);
			}
			THEN("There should be nothing to detach")
			{
				REQUIRE(modular->detach({ 3, -1 }) == nullptr);
			}
		}
		WHEN("We attach and destroy a module")
		{
			let turret_pos = vi32{ 1, -1 };
			std::unique_ptr<interface::item> turret =
				std::make_unique<module::turret>(rm, random, [&] {
				YAML::Node _;
				_["health"] = 100.f;
				_["armor"] = 0.f;
				return _;
			}());
			REQUIRE(success(modular->attach(turret_pos, move(turret))));
			std::unordered_set<gsl::not_null<interface::damageable*>>
				damaged, knocked;
			random_stub random;
			let is_damaged = modular->module_at(turret_pos)->damage([&] {
				interface::damageable::info _;
				_.physical = 100.f;
				_.knocked = &knocked;
				_.damaged = &damaged;
				_.team = 1;
				_.random = random;
				return _;
			}());
			THEN("damage should return true")
			{
				REQUIRE(is_damaged);
			}
			THEN(
				"module should show up as 'dead', but not be deleted instantly")
			{
				let module = modular->module_at(turret_pos);
				REQUIRE(module);
				REQUIRE(module->dead());
			}
			THEN("calling world.update should delete the module from the "
				 "modular")
			{
				update_context context(rm, random);
				context.dt = 0.15;
				world.update(context, {});
				REQUIRE(modular->at(turret_pos) == nullptr);
				REQUIRE(modular->module_at(turret_pos) == nullptr);
			}
		}
		WHEN("We try to push an item into a modular with no containers")
		{
			std::unique_ptr<interface::item> shard =
				std::make_unique<item::shard>(rm);
			let error_code = push(*modular, move(shard));
			THEN("push returns a failure")
			{
				REQUIRE(failure(error_code));
			}
		}
		WHEN("We push an item into a modular with an empty container")
		{
			std::unique_ptr<module::base> module =
				std::make_unique<module::cargo>(rm, random, YAML::Node());
			REQUIRE(success(modular->attach({ 1, -1 }, move(module))));
			REQUIRE(!modular->containers().empty());
			std::unique_ptr<interface::item> shard =
				std::make_unique<item::shard>(rm);
			let& shard_ref = *shard;
			let error_code = push(*modular, move(shard));
			THEN("push returns a success")
			{
				REQUIRE(success(error_code));
			}
			THEN("empties the pointer passed into push")
			{
				REQUIRE(shard == nullptr);
			}
			THEN("push puts the item at the 0, 0 position")
			{
				REQUIRE(
					modular->containers().front().get().at({ 0, 0 })
					== &shard_ref);
			}
		}
		WHEN("We push two identical stackable items into a modular with an "
			 "empty container")
		{
			std::unique_ptr<module::base> module =
				std::make_unique<module::cargo>(rm, random, YAML::Node());
			REQUIRE(success(modular->attach({ 1, -1 }, move(module))));
			REQUIRE(!modular->containers().empty());
			auto shard_ptr = std::make_unique<item::shard>(rm);
			let& shard = *shard_ptr;
			REQUIRE(success(push(*modular, move(shard_ptr))));
			std::unique_ptr<interface::item> second_shard =
				std::make_unique<item::shard>(rm);
			let error_code = push(*modular, move(second_shard));
			THEN("second push returns a success")
			{
				REQUIRE(error_code == error::code::stacked);
			}
			THEN("push puts first item at the 0, 0 position")
			{
				REQUIRE(
					modular->containers().front().get().at({ 0, 0 })
					== &shard);
			}
			THEN("push destroys the second item")
			{
				REQUIRE(second_shard == nullptr);
			}
			THEN("increasing the quantity on the first to two")
			{
				REQUIRE(
					modular->containers().front().get().at({ 0, 0 })->quantity()
					== 2);
			}
		}
		WHEN("We put a stackable item into a modular with a single container "
			 "that already has an item of this kind in the bottom-right corner")
		{
			std::unique_ptr<module::base> module =
				std::make_unique<module::cargo>(rm, random, YAML::Node());
			REQUIRE(success(modular->attach({ 1, -1 }, move(module))));
			REQUIRE(!modular->containers().empty());
			REQUIRE(success(modular->containers().front().get().attach(
				{ 15, 3 }, std::make_unique<item::shard>(rm))));
			std::unique_ptr<interface::item> second_shard =
				std::make_unique<item::shard>(rm);
			let error_code = push(*modular, move(second_shard));
			THEN("push returns a success")
			{
				REQUIRE(error_code == error::code::stacked);
			}
			THEN("slot [0, 0] of the container remains empty")
			{
				REQUIRE(
					modular->containers().front().get().at({ 0, 0 })
					== nullptr);
			}
			THEN("push destroys the second item increasing the quantity on the "
				 "first to two");
			{
				REQUIRE(second_shard == nullptr);
				REQUIRE(
					modular->containers().front().get().at({ 15, 3 })->quantity()
					== 2);
			}
		}
	}
}

