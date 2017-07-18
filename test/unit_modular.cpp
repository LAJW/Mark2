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
