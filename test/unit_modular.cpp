#include "../game/resource_manager.h"
#include "../game/exception.h"
#include "../game/module_core.h"
#include "../game/module_turret.h"
#include "../game/tick_context.h"
#include "../game/unit_modular.h"
#include "../game/world.h"
#include "../game/world_stack.h"
#include <catch.hpp>

using namespace mark;

TEST_CASE("Create an empty modular") {
	resource::manager_stub rm;
	world world(rm);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = &world;
		_.pos = { 1, 2 };
		_.rotation = 3.f;
		return _;
	}());
	REQUIRE(modular.pos() == vector<double>(1, 2));
	REQUIRE(modular.rotation() == 3.f);
}

TEST_CASE("Create modular with a core") {
	resource::manager_stub rm;
	world world(rm);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = &world;
		_.pos = { 1, 2 };
		_.rotation = 3.f;
		return _;
	}());
	std::unique_ptr<module::base> core =
		std::make_unique<module::core>(rm);
	let& core_ref = *core;
	REQUIRE(modular.attach({ -1, -1 }, core) == error::code::success);
	REQUIRE(core == nullptr);
	REQUIRE(core_ref.pos() == vector<double>(1, 2));
	REQUIRE(core_ref.grid_pos() == vector<int>(-1, -1));
	REQUIRE(modular.at({ -1, -1 }) == &core_ref);
	REQUIRE(modular.at({ 0, -1 }) == &core_ref);
	REQUIRE(modular.at({ -1, 0 }) == &core_ref);
	REQUIRE(modular.at({ 0, 0 }) == &core_ref);
	REQUIRE(modular.at({ 1, -1 }) == nullptr);
}

TEST_CASE("Try creating modular with two cores") {
	resource::manager_stub rm;
	world world(rm);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = &world;
		return _;
	}());
	std::unique_ptr<module::base> core1 =
		std::make_unique<module::core>(rm);
	std::unique_ptr<module::base> core2 =
		std::make_unique<module::core>(rm);
	REQUIRE(error::code::success == modular.attach({ -1, -1 }, core1));
	REQUIRE(error::code::success != modular.attach({ 1, -1 }, core2));
}

TEST_CASE("Create modular with a turret to the right") {
	resource::manager_stub rm;
	world world(rm);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = &world;
		return _;
	}());
	std::unique_ptr<module::base> core =
		std::make_unique<module::core>(rm);
	REQUIRE(error::code::success == modular.attach({ -1, -1 }, core));

	std::unique_ptr<module::base> turret = std::make_unique<module::turret>([&] {
		module::turret::info _;
		_.resource_manager = &rm;
		return _;
	}());
	let& turret_ref = *turret;
	REQUIRE(error::code::success == modular.attach({ 1, -1 }, turret));
	REQUIRE(turret_ref.grid_pos() == vector<int>(1, -1));
	REQUIRE(turret_ref.pos().x == Approx(module::size * 2));
	REQUIRE(turret_ref.pos().y == Approx(0));
}

TEST_CASE("Attach turret in all possible positions") {
	resource::manager_stub rm;
	world world(rm);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = &world;
		return _;
	}());
	std::unique_ptr<module::base> core =
		std::make_unique<module::core>(rm);
	REQUIRE(error::code::success == modular.attach({ -1, -1 }, core));

	std::unique_ptr<module::base> turret = std::make_unique<module::turret>([&] {
		module::turret::info _;
		_.resource_manager = &rm;
		return _;
	}());
	let& turret_ref = *turret;

	// Right
	REQUIRE(error::code::success == modular.attach({ 1, -1 }, turret));
	turret = modular.detach({ 1, -1 });

	REQUIRE(error::code::success == modular.attach({ 1, -2 }, turret));
	turret = modular.detach({ 1, -2 });

	REQUIRE(error::code::success == modular.attach({ 1, 0 }, turret));
	turret = modular.detach({ 1, 0 });

	// Left
	REQUIRE(error::code::success == modular.attach({ -3, -1 }, turret));
	turret = modular.detach({ -3, -1 });

	REQUIRE(error::code::success == modular.attach({ -3, -2 }, turret));
	turret = modular.detach({ -3, -2 });

	REQUIRE(error::code::success == modular.attach({ -3, 0 }, turret));
	turret = modular.detach({ -3, 0 });

	// Top
	REQUIRE(error::code::success == modular.attach({ -1, 1 }, turret));
	turret = modular.detach({ -1, 1 });

	REQUIRE(error::code::success == modular.attach({ -2, 1 }, turret));
	turret = modular.detach({ -2, 1 });

	REQUIRE(error::code::success == modular.attach({ 0, 1 }, turret));
	turret = modular.detach({ 0, 1 });

	// Left
	REQUIRE(error::code::success == modular.attach({ -1, -3 }, turret));
	turret = modular.detach({ -1, -3 });

	REQUIRE(error::code::success == modular.attach({ -2, -3 }, turret));
	turret = modular.detach({ -2, -3 });

	REQUIRE(error::code::success == modular.attach({ 0, -3 }, turret));
	turret = modular.detach({ 0, -3 });
}

TEST_CASE("Failed detached attach should not set internal map") {
	resource::manager_stub rm;
	world world(rm);
	unit::modular modular([&] {
		unit::modular::info _;
		_.world = &world;
		return _;
	}());
	std::unique_ptr<module::base> core =
		std::make_unique<module::core>(rm);
	REQUIRE(error::code::success == modular.attach({ -1, -1 }, core));

	std::unique_ptr<module::base> turret = std::make_unique<module::turret>([&] {
		module::turret::info _;
		_.resource_manager = &rm;
		return _;
	}());
	let& turret_ref = *turret;

	REQUIRE(modular.at({ 3, -1 }) == nullptr);
	REQUIRE(error::code::success != modular.attach({ 3, -1 }, turret));
	turret = modular.detach({ 3, -1 });
	REQUIRE(modular.at({ 3, -1 }) == nullptr);
}

TEST_CASE("Remove dead modules") {
	resource::manager_stub rm;
	world world(rm);
	auto modular = std::make_shared<unit::modular>([&] {
		unit::modular::info _;
		_.world = &world;
		return _;
	}());
	world.attach(modular);
	std::unique_ptr<module::base> core =
		std::make_unique<module::core>(rm);
	REQUIRE(error::code::success == modular->attach({ -1, -1 }, core));

	std::unique_ptr<module::base> turret = std::make_unique<module::turret>([&] {
		module::turret::info _;
		_.cur_health = 0.f;
		_.resource_manager = &rm;
		return _;
	}());
	REQUIRE(error::code::success == modular->attach({ 1, -1 }, turret));
	tick_context context(rm);
	context.dt = 0.15;
	world.tick(context, { });
	REQUIRE(modular->at({ 1, -1 }) == nullptr);
}
