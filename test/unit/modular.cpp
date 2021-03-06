﻿#include <algorithm/find_if.h>
#include <catch.hpp>
#include <exception.h>
#include <item/shard.h>
#include <module/cargo.h>
#include <module/core.h>
#include <module/turret.h>
#include <random.h>
#include <ref.h>
#include <resource/manager.h>
#include <unit/impl/modular.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>
#include <world_stack.h>

using namespace mark;

TEST_CASE("Create an empty modular")
{
	resource::manager_stub rm;
	random_stub random;
	auto world = mark::world(ref(rm), ref(random));
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
	auto world = mark::world(ref(rm), ref(random));
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
	REQUIRE(modular.at({ -1, -1 })->equals(core_ref));
	REQUIRE(modular.at({ 0, -1 })->equals(core_ref));
	REQUIRE(modular.at({ -1, 0 })->equals(core_ref));
	REQUIRE(modular.at({ 0, 0 })->equals(core_ref));
	REQUIRE(!modular.at({ 1, -1 }).has_value());
}

TEST_CASE("Try creating modular with two cores")
{
	resource::manager_stub rm;
	random_stub random;
	auto world = mark::world(ref(rm), ref(random));
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
	auto world = mark::world(ref(rm), ref(random));
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
	auto world = mark::world(ref(rm), ref(random));
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
		auto world = mark::world(ref(rm), ref(random));
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
				REQUIRE(!modular->at({ 3, -1 }).has_value());
			}
			THEN("Out-of-bounds should return a nullptr")
			{
				REQUIRE(!modular->at({ -21, 0 }).has_value());
				REQUIRE(!modular->module_at({ -21, 0 }).has_value());
				REQUIRE(!modular->at({ -22, -22 }).has_value());
				REQUIRE(!modular->module_at({ -22, -22 }).has_value());
				REQUIRE(!modular->at({ 0, -22 }).has_value());
				REQUIRE(!modular->module_at({ 0, -22 }).has_value());
				REQUIRE(!modular->at({ 22, -22 }).has_value());
				REQUIRE(!modular->module_at({ 22, -22 }).has_value());
				REQUIRE(!modular->at({ 22, 22 }).has_value());
				REQUIRE(!modular->module_at({ 22, 22 }).has_value());
				REQUIRE(!modular->at({ 0, 22 }).has_value());
				REQUIRE(!modular->module_at({ 0, 22 }).has_value());
				REQUIRE(!modular->at({ 22, 0 }).has_value());
				REQUIRE(!modular->module_at({ 22, 0 }).has_value());
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
				REQUIRE(!modular->at({ 3, -1 }).has_value());
				REQUIRE(!modular->module_at({ 3, -1 }).has_value());
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
			std::unordered_set<gsl::not_null<interface::damageable*>> damaged,
				knocked;
			random_stub random;
			let is_damaged = modular->module_at(turret_pos)->damage([&] {
				interface::damageable::info _;
				_.physical = 100.f;
				_.knocked = knocked;
				_.damaged = damaged;
				_.team = 1;
				_.random = random;
				return _;
			}());
			THEN("damage should return true") { REQUIRE(is_damaged); }
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
				REQUIRE(!modular->at(turret_pos).has_value());
				REQUIRE(!modular->module_at(turret_pos).has_value());
			}
		}
		WHEN("We try to push an item into a modular with no containers")
		{
			std::unique_ptr<interface::item> shard =
				std::make_unique<item::shard>(rm);
			let error_code = push(*modular, move(shard));
			THEN("push returns a failure") { REQUIRE(failure(error_code)); }
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
			THEN("push returns a success") { REQUIRE(success(error_code)); }
			THEN("empties the pointer passed into push")
			{
				REQUIRE(shard == nullptr);
			}
			THEN("push puts the item at the 0, 0 position")
			{
				REQUIRE(
					modular->containers().front().get().at({ 0, 0 })->equals(
						shard_ref));
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
					modular->containers().front().get().at({ 0, 0 })->equals(
						shard));
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
			auto& first_container = modular->containers().front().get();
			REQUIRE(success(first_container.attach(
				{ 15, 3 }, std::make_unique<item::shard>(rm))));
			std::unique_ptr<interface::item> second_shard =
				std::make_unique<item::shard>(rm);
			let error_code = push(*modular, move(second_shard));
			THEN("push returns `stacked`")
			{
				REQUIRE(error_code == error::code::stacked);
			}
			THEN("slot [0, 0] of the container remains empty")
			{
				REQUIRE(!first_container.at({ 0, 0 }).has_value());
			}
			THEN("push destroys the second item increasing the quantity on the "
				 "first to two")
			{
				REQUIRE(second_shard == nullptr);
				REQUIRE(first_container.at({ 15, 3 })->quantity() == 2);
			}
		}
		WHEN(
			"Pushing an item into a modular with two containers, where the "
			"second container contains an existing stack should store it there")
		{
			REQUIRE(success(modular->attach(
				{ 1, -1 },
				std::unique_ptr<module::base>(std::make_unique<module::cargo>(
					rm, random, YAML::Node())))));
			REQUIRE(success(modular->attach(
				{ 1, 1 },
				std::unique_ptr<module::base>(std::make_unique<module::cargo>(
					rm, random, YAML::Node())))));
			REQUIRE(modular->containers().size() == 2);
			auto& second_container = modular->containers().back().get();
			REQUIRE(success(second_container.attach(
				{ 15, 3 }, std::make_unique<item::shard>(rm))));
			std::unique_ptr<interface::item> second_shard =
				std::make_unique<item::shard>(rm);
			let error_code = push(*modular, move(second_shard));
			THEN("push returns `stacked`")
			{
				REQUIRE(error_code == error::code::stacked);
			}
			THEN("slot [0, 0] of the container remains empty")
			{
				REQUIRE(!second_container.at({ 0, 0 }).has_value());
			}
			THEN("push destroys the second item increasing the quantity on the "
				 "first to two")
			{
				REQUIRE(second_shard == nullptr);
				REQUIRE(second_container.at({ 15, 3 })->quantity() == 2);
			}
		}
	}
}

SCENARIO("Implementation functions")
{
	using namespace mark::unit::impl;
	GIVEN("Limit angle function")
	{
		WHEN("supplied with 0.")
		{
			THEN("it should return 0.") { REQUIRE(0 == limit_angle(0.)); }
		}
		WHEN("supplied with +30.")
		{
			THEN("it should return +30.") { REQUIRE(30. == limit_angle(30.)); }
		}
		WHEN("supplied with -30.")
		{
			THEN("it should return -30.")
			{
				REQUIRE(-30. == limit_angle(-30.));
			}
		}
		WHEN("supplied with -270.")
		{
			THEN("it should return 90.") { REQUIRE(90. == limit_angle(-270.)); }
		}
		WHEN("supplied with 270.")
		{
			THEN("it should return -90.")
			{
				REQUIRE(-90. == limit_angle(270.));
			}
		}
	}
	GIVEN("should_accelerate function")
	{
		WHEN("North-East into South-West zero speed")
		{
			should_accelerate_info _;
			_.rotation = 15.;
			_.target_angle = -175;
			_.angular_acceleration = 100.;
			_.angular_velocity = 0.;
			THEN("Should return true")
			{
				REQUIRE(should_accelerate(_) == true);
			}
		}
		WHEN("North-West into South-West godspeed")
		{
			should_accelerate_info _;
			_.rotation = 160.;
			_.target_angle = -175;
			_.angular_acceleration = 100.;
			_.angular_velocity = 90.;
			THEN("Should return true")
			{
				REQUIRE(should_accelerate(_) == false);
			}
		}
	}
	GIVEN("rotation_and_angular_velocity_function")
	{
		WHEN("NE->SW Accelerate from zero")
		{
			rotation_and_angular_velocity_info _;
			_.rotation = 20.;
			_.angular_velocity = 0.;
			_.angular_acceleration = 100.;
			_.dt = .16;
			_.target = { -100., -30. };
			let[rotation, angular_velocity] = rotation_and_angular_velocity(_);
			THEN("Should accelerate") { REQUIRE(angular_velocity > 0.); }
			THEN("Rotation should be between 20 and 90 degrees, but not 20")
			{
				REQUIRE(rotation > 20.);
				REQUIRE(rotation < 90.);
			}
		}
		WHEN("SE->NW Accelerate from zero")
		{
			rotation_and_angular_velocity_info _;
			_.rotation = -20.;
			_.angular_velocity = 0.;
			_.angular_acceleration = 100.;
			_.dt = .16;
			_.target = { -100., 30. };
			let[rotation, angular_velocity] = rotation_and_angular_velocity(_);
			THEN("Speed should be negative") { REQUIRE(angular_velocity > 0.); }
			THEN("Rotation should be between -20 and -90 degrees, but not 20")
			{
				REQUIRE(rotation < -20.);
				REQUIRE(rotation > -90.);
			}
		}
		WHEN("NW->SW Decelerate")
		{
			rotation_and_angular_velocity_info _;
			_.rotation = 160.;
			_.angular_velocity = 90.;
			_.angular_acceleration = 100.;
			_.dt = .16;
			_.target = { -100., -30. };
			let[rotation, angular_velocity] = rotation_and_angular_velocity(_);
			THEN("Should decelerate") { REQUIRE(angular_velocity < 90.); }
			THEN("Rotation should be between 165 and 180 degrees")
			{
				let top_chunk = rotation > 165. && rotation <= 180.;
				REQUIRE(top_chunk);
			}
		}
		WHEN("SE->NW Decelerate")
		{
			rotation_and_angular_velocity_info _;
			_.rotation = -160.;
			_.angular_velocity = 90.;
			_.angular_acceleration = 100.;
			_.dt = .16;
			_.target = { -100., 30. };
			let[rotation, angular_velocity] = rotation_and_angular_velocity(_);
			THEN("Should decelerate") { REQUIRE(angular_velocity < 90.); }
			THEN("Rotation should be between 165 and -180 degrees, excluding")
			{
				let bottom_chunk = rotation < -165. && rotation > -180.;
				REQUIRE(bottom_chunk);
			}
		}
		WHEN("NE->SW Full acceleration simulation")
		{
			double rotation = 0.;
			double angular_velocity = 0;
			size_t remaining_iterations = 1000;
			std::vector<double> velocities = { 0 };
			bool was_accelerating = false;
			while (rotation < 179.5) {
				if (--remaining_iterations == 0) {
					REQUIRE(false); // The loop should stop
				}
				rotation_and_angular_velocity_info _;
				_.rotation = rotation;
				_.angular_velocity = angular_velocity;
				_.angular_acceleration = 200.;
				_.dt = .08;
				_.target = { -100., -0. };
				let[new_rotation, new_angular_velocity] =
					rotation_and_angular_velocity(_);
				REQUIRE(new_rotation >= rotation);
				if (rotation < 90.) {
					REQUIRE(new_angular_velocity > angular_velocity);
				} else if (rotation > 90.) {
					if (new_angular_velocity > angular_velocity) {
						if (was_accelerating) {
							// Accelerating twice in a row. Likely decelerating
							// too quickly
							REQUIRE(false);
						} else {
							was_accelerating = true;
						}
					} else {
						was_accelerating = false;
					}
				}
				rotation = new_rotation;
				angular_velocity = new_angular_velocity;
				velocities.push_back(angular_velocity);
			}
			THEN("The goal should be reached") { REQUIRE(rotation == 180.); }
			THEN("The angular velocity should be set to zero")
			{
				REQUIRE(angular_velocity == 0.);
			}
			THEN(
				"Average deceleration and acceleration should be roughly equal")
			{
				std::vector<double> accelerations;
				REQUIRE(velocities.size() > 3);
				for (auto it = next(velocities.begin()); it != velocities.end();
					 ++it) {
					accelerations.push_back(*it - *prev(it));
				}
				auto middle = find_if(accelerations, [&](let acceleration) {
					return acceleration < 0.;
				});
				REQUIRE(middle != accelerations.end());
				let average_acceleration =
					std::accumulate(accelerations.begin(), middle, 0.)
					/ gsl::narrow_cast<double>(middle - accelerations.begin());
				let average_deceleration =
					std::accumulate(middle, accelerations.end(), 0.)
					/ gsl::narrow_cast<double>(accelerations.end() - middle);
				REQUIRE(abs(average_acceleration + average_deceleration) < 1.5);
			}
		}
	}
}
