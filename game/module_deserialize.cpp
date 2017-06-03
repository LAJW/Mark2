#include "exception.h"
#include "module_battery.h"
#include "module_cannon.h"
#include "module_cargo.h"
#include "module_core.h"
#include "module_energy_generator.h"
#include "module_engine.h"
#include "module_flamethrower.h"
#include "module_shield_generator.h"
#include "module_turret.h"

auto mark::module::deserialize(mark::resource::manager& rm, const YAML::Node& node) ->
	std::unique_ptr<mark::module::base> {
	const auto type = node["type"].as<std::string>();
	if (type == "module_battery") {
		return std::make_unique<mark::module::battery>(rm, node);
	} else if (type == "module_cannon") {
		return std::make_unique<mark::module::cannon>(rm, node);
	} else if (type == "module_cargo") {
		return std::make_unique<mark::module::cargo>(rm, node);
	} else if (type == "module_core") {
		return std::make_unique<mark::module::core>(rm, node);
	} else if (type == "module_energy_generator") {
		return std::make_unique<mark::module::energy_generator>(rm, node);
	} else if (type == "module_engine") {
		return std::make_unique<mark::module::engine>(rm, node);
	} else if (type == "module_flamethrower") {
		return std::make_unique<mark::module::flamethrower>(rm, node);
	} else if (type == "module_shield_generator") {
		return std::make_unique<mark::module::shield_generator>(rm, node);
	} else if (type == "module_turret") {
		return std::make_unique<mark::module::turret>(rm, node);
	} else {
		throw mark::exception("UNKNOWN_MODULE");
	}
}

