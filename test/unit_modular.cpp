#include "../game/resource_manager.h"
#include "../game/world.h"
#include "../game/unit_modular.h"
#include "../game/module_core.h"
#include "../game/module_turret.h"
#include "../game/exception.h"
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
	modular.attach(core, { -1, -1 });
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
	modular.attach(core1, { -1, -1 });
	try {
		modular.attach(core2, { 1, -1 });
		REQUIRE(false);
	} catch (const mark::exception&) {
		REQUIRE(core2 != nullptr);
	}
}

TEST_CASE("Create modular with a turret to the right") {
	mark::resource::manager_stub rm;
	mark::world world(rm, true);
	mark::unit::modular modular(world, { 0, 0 }, 0);
	std::unique_ptr<mark::module::base> core =
		std::make_unique<mark::module::core>(rm);
	modular.attach(core, { -1, -1 });

	mark::module::turret::info info;
	info.resource_manager = &rm;
	std::unique_ptr<mark::module::base> turret =
		std::make_unique<mark::module::turret>(info);
	const auto& turret_ref = *turret;

	modular.attach(turret, { 1, -1 });
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
	modular.attach(core, { -1, -1 });

	mark::module::turret::info info;
	info.resource_manager = &rm;
	std::unique_ptr<mark::module::base> turret =
		std::make_unique<mark::module::turret>(info);
	const auto& turret_ref = *turret;

	// Right
	modular.attach(turret, { 1, -1 });
	turret = modular.detach({ 1, -1 });

	modular.attach(turret, { 1, -2 });
	turret = modular.detach({ 1, -2 });

	modular.attach(turret, { 1, 0 });
	turret = modular.detach({ 1, 0 });

	// Left
	modular.attach(turret, { -3, -1 });
	turret = modular.detach({ -3, -1 });

	modular.attach(turret, { -3, -2 });
	turret = modular.detach({ -3, -2 });

	modular.attach(turret, { -3, 0 });
	turret = modular.detach({ -3, 0 });

	// Top
	modular.attach(turret, { -1, 1 });
	turret = modular.detach({ -1, 1 });

	modular.attach(turret, { -2, 1 });
	turret = modular.detach({ -2, 1 });

	modular.attach(turret, { 0, 1 });
	turret = modular.detach({ 0, 1 });

	// Left
	modular.attach(turret, { -1, -3 });
	turret = modular.detach({ -1, -3 });

	modular.attach(turret, { -2, -3 });
	turret = modular.detach({ -2, -3 });

	modular.attach(turret, { 0, -3 });
	turret = modular.detach({ 0, -3 });
}

TEST_CASE("Failed detached attach should not set internal map") {
	mark::resource::manager_stub rm;
	mark::world world(rm, true);
	mark::unit::modular modular(world, { 0, 0 }, 0);
	std::unique_ptr<mark::module::base> core =
		std::make_unique<mark::module::core>(rm);
	modular.attach(core, { -1, -1 });

	mark::module::turret::info info;
	info.resource_manager = &rm;
	std::unique_ptr<mark::module::base> turret =
		std::make_unique<mark::module::turret>(info);
	const auto& turret_ref = *turret;

	REQUIRE(modular.at({ 3, -1 }) == nullptr);
	try {
		modular.attach(turret, { 3, -1 });
	} catch (const std::exception&) { /* no-op */ }
	turret = modular.detach({ 3, -1 });
	REQUIRE(modular.at({ 3, -1 }) == nullptr);
}
