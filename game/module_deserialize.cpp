#include "stdafx.h"
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
	if (type == mark::module::battery::type_name) {
		return std::make_unique<mark::module::battery>(rm, node);
	} else if (type == mark::module::cannon::type_name) {
		return std::make_unique<mark::module::cannon>(rm, node);
	} else if (type == mark::module::cargo::type_name) {
		return std::make_unique<mark::module::cargo>(rm, node);
	} else if (type == mark::module::core::type_name) {
		return std::make_unique<mark::module::core>(rm, node);
	} else if (type == mark::module::energy_generator::type_name) {
		return std::make_unique<mark::module::energy_generator>(rm, node);
	} else if (type == mark::module::engine::type_name) {
		return std::make_unique<mark::module::engine>(rm, node);
	} else if (type == mark::module::flamethrower::type_name) {
		return std::make_unique<mark::module::flamethrower>(rm, node);
	} else if (type == mark::module::shield_generator::type_name) {
		return std::make_unique<mark::module::shield_generator>(rm, node);
	} else if (type == mark::module::turret::type_name) {
		return std::make_unique<mark::module::turret>(rm, node);
	} else {
		throw mark::exception("UNKNOWN_MODULE");
	}
}

