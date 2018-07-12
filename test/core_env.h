#pragma once
#include <module/core.h>
#include <resource_manager.h>
#include <unit/modular.h>
#include <world.h>

// Module environment - a helper struct for with a world and a modular with a
// core
struct core_env
{
	mark::resource::manager_stub rm;
	mark::world world;
	std::shared_ptr<mark::unit::modular> modular;
	mark::module::core& core;
	core_env()
		: world(rm)
		, modular([&] {
			mark::unit::modular::info info;
			info.world = world;
			auto modular = std::make_shared<mark::unit::modular>(info);
			std::unique_ptr<mark::interface::item> core =
				std::make_unique<mark::module::core>(rm, YAML::Node());
			Expects(!modular->attach({ -1, -1 }, move(core)));
			world.attach(modular);
			return modular;
		}())
		, core(dynamic_cast<mark::module::core&>(
			  *modular->module_at(mark::vi32(0, 0))))
	{}
};