#include "../game/resource_manager.h"
#include "../game/world.h"
#include "../game/unit_modular.h"
#include "../game/module_core.h"
#include "../game/module_turret.h"
#include "../game/exception.h"
#include "../game/tick_context.h"
#include <catch.hpp>

TEST_CASE("Create an empty modular") {
	mark::resource::manager_stub rm;
	mark::world world(rm, true);
	mark::unit::modular modular(world, { 1, 2 }, 3);
	REQUIRE(modular.pos() == mark::vector<double>(1, 2));
	REQUIRE(modular.rotation() == 3.0);
}

TEST_CASE("Create modular with a core") {
	mark::resource::manager_stub rm;
	mark::world world(rm, true);
	mark::unit::modular modular(world, { 1, 2 }, 3);
	std::unique_ptr<mark::module::base> core =
		std::make_unique<mark::module::core>(rm);
	const auto& core_ref = *core;
	REQUIRE(modular.attach({ -1, -1 }, core) == mark::error::code::success);
	REQUIRE(core == nullptr);
	REQUIRE(core_ref.pos() == mark::vector<double>(1, 2));
	REQUIRE(core_ref.grid_pos() == mark::vector<int>(-1, -1));
	REQUIRE(modular.at({ -1, -1 }) == &core_ref);
	REQUIRE(modular.at({ 0, -1 }) == &core_ref);
	REQUIRE(modular.at({ -1, 0 }) == &core_ref);
	REQUIRE(modular.at({ 0, 0 }) == &core_ref);
	REQUIRE(modular.at({ 1, -1 }) == nullptr);
}

TEST_CASE("Try creating modular with two cores") {
	mark::resource::manager_stub rm;
	mark::world world(rm, true);
	mark::unit::modular modular(world, { 1, 2 }, 3);
	std::unique_ptr<mark::module::base> core1 =
		std::make_unique<mark::module::core>(rm);
	std::unique_ptr<mark::module::base> core2 =
		std::make_unique<mark::module::core>(rm);
	REQUIRE(mark::error::code::success == modular.attach({ -1, -1 }, core1));
	REQUIRE(mark::error::code::success != modular.attach({ 1, -1 }, core2));
}

TEST_CASE("Create modular with a turret to the right") {
	mark::resource::manager_stub rm;
	mark::world world(rm, true);
	mark::unit::modular modular(world, { 0, 0 }, 0);
	std::unique_ptr<mark::module::base> core =
		std::make_unique<mark::module::core>(rm);
	REQUIRE(mark::error::code::success == modular.attach({ -1, -1 }, core));

	mark::module::turret::info info;
	info.resource_manager = &rm;
	std::unique_ptr<mark::module::base> turret =
		std::make_unique<mark::module::turret>(info);
	const auto& turret_ref = *turret;

	REQUIRE(mark::error::code::success == modular.attach({ 1, -1 }, turret));
	REQUIRE(turret_ref.grid_pos() == mark::vector<int>(1, -1));
	REQUIRE(turret_ref.pos().x == Approx(mark::module::size * 2));
	REQUIRE(turret_ref.pos().y == Approx(0));
}

TEST_CASE("Attach turret in all possible positions") {
	mark::resource::manager_stub rm;
	mark::world world(rm, true);
	mark::unit::modular modular(world, { 0, 0 }, 0);
	std::unique_ptr<mark::module::base> core =
		std::make_unique<mark::module::core>(rm);
	REQUIRE(mark::error::code::success == modular.attach({ -1, -1 }, core));

	mark::module::turret::info info;
	info.resource_manager = &rm;
	std::unique_ptr<mark::module::base> turret =
		std::make_unique<mark::module::turret>(info);
	const auto& turret_ref = *turret;

	// Right
	REQUIRE(mark::error::code::success == modular.attach({ 1, -1 }, turret));
	turret = modular.detach({ 1, -1 });

	REQUIRE(mark::error::code::success == modular.attach({ 1, -2 }, turret));
	turret = modular.detach({ 1, -2 });

	REQUIRE(mark::error::code::success == modular.attach({ 1, 0 }, turret));
	turret = modular.detach({ 1, 0 });

	// Left
	REQUIRE(mark::error::code::success == modular.attach({ -3, -1 }, turret));
	turret = modular.detach({ -3, -1 });

	REQUIRE(mark::error::code::success == modular.attach({ -3, -2 }, turret));
	turret = modular.detach({ -3, -2 });

	REQUIRE(mark::error::code::success == modular.attach({ -3, 0 }, turret));
	turret = modular.detach({ -3, 0 });

	// Top
	REQUIRE(mark::error::code::success == modular.attach({ -1, 1 }, turret));
	turret = modular.detach({ -1, 1 });

	REQUIRE(mark::error::code::success == modular.attach({ -2, 1 }, turret));
	turret = modular.detach({ -2, 1 });

	REQUIRE(mark::error::code::success == modular.attach({ 0, 1 }, turret));
	turret = modular.detach({ 0, 1 });

	// Left
	REQUIRE(mark::error::code::success == modular.attach({ -1, -3 }, turret));
	turret = modular.detach({ -1, -3 });

	REQUIRE(mark::error::code::success == modular.attach({ -2, -3 }, turret));
	turret = modular.detach({ -2, -3 });

	REQUIRE(mark::error::code::success == modular.attach({ 0, -3 }, turret));
	turret = modular.detach({ 0, -3 });
}

TEST_CASE("Failed detached attach should not set internal map") {
	mark::resource::manager_stub rm;
	mark::world world(rm, true);
	mark::unit::modular modular(world, { 0, 0 }, 0);
	std::unique_ptr<mark::module::base> core =
		std::make_unique<mark::module::core>(rm);
	REQUIRE(mark::error::code::success == modular.attach({ -1, -1 }, core));

	mark::module::turret::info info;
	info.resource_manager = &rm;
	std::unique_ptr<mark::module::base> turret =
		std::make_unique<mark::module::turret>(info);
	const auto& turret_ref = *turret;

	REQUIRE(modular.at({ 3, -1 }) == nullptr);
	REQUIRE(mark::error::code::success != modular.attach({ 3, -1 }, turret));
	turret = modular.detach({ 3, -1 });
	REQUIRE(modular.at({ 3, -1 }) == nullptr);
}

TEST_CASE("Remove dead modules") {
	mark::resource::manager_stub rm;
	mark::world world(rm, true);
	mark::unit::modular modular(world, { 0, 0 }, 0);
	std::unique_ptr<mark::module::base> core =
		std::make_unique<mark::module::core>(rm);
	REQUIRE(mark::error::code::success == modular.attach({ -1, -1 }, core));

	mark::module::turret::info info;
	info.cur_health = 0.f;
	info.resource_manager = &rm;
	std::unique_ptr<mark::module::base> turret =
		std::make_unique<mark::module::turret>(info);
	REQUIRE(mark::error::code::success == modular.attach({ 1, -1 }, turret));
	mark::tick_context context(rm);
	context.dt = 0.15;
	modular.tick(context);
	REQUIRE(modular.at({ 1, -1 }) == nullptr);
}
