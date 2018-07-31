#pragma once
#include <module/core.h>
#include <random.h>
#include <resource_manager.h>
#include <unit/modular.h>
#include <world.h>

/// make_unique with cast to the mark::interface::item base class
template <typename T, typename... Ts>
std::unique_ptr<mark::interface::item> make_item(Ts&&... ts)
{
	return std::make_unique<T>(std::forward<Ts>(ts)...);
}

// Module environment - a helper struct for with a world and a modular with a
// core
struct core_env
{
	mark::resource::manager_stub rm;
	mark::random_stub random;
	mark::world world;
	std::shared_ptr<mark::unit::modular> modular;
	mark::module::core& core;
	core_env()
		: world(rm, random)
		, modular([&] {
			mark::unit::modular::info info;
			info.world = world;
			auto modular = std::make_shared<mark::unit::modular>(info);
			auto core = make_item<mark::module::core>(rm, random, YAML::Node());
			Expects(!modular->attach({ -1, -1 }, move(core)));
			world.attach(modular);
			return modular;
		}())
		, core(dynamic_cast<mark::module::core&>(
			  *modular->module_at(mark::vi32(0, 0))))
	{}
};
