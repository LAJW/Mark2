#include <stdafx.h>
#include "armor.h"
#include "battery.h"
#include "laser.h"
#include "cargo.h"
#include "core.h"
#include "energy_generator.h"
#include "engine.h"
#include "flamethrower.h"
#include "healing_turret.h"
#include "shield_generator.h"
#include "turret.h"
#include "warp_drive.h"
#include <exception.h>

auto mark::module::deserialize(
	resource::manager& rm,
	random& random,
	const YAML::Node& node)
	-> module::base_ptr
{
	let type = node["type"].as<std::string>();
	if (type == module::armor::type_name) {
		return std::make_unique<module::armor>(rm, random, node);
	}
	if (type == module::battery::type_name) {
		return std::make_unique<module::battery>(rm, random, node);
	}
	if (type == module::laser::type_name) {
		return std::make_unique<module::laser>(rm, random, node);
	}
	if (type == module::cargo::type_name) {
		return std::make_unique<module::cargo>(rm, random, node);
	}
	if (type == module::core::type_name) {
		return std::make_unique<module::core>(rm, random, node);
	}
	if (type == module::energy_generator::type_name) {
		return std::make_unique<module::energy_generator>(rm, random, node);
	}
	if (type == module::engine::type_name) {
		return std::make_unique<module::engine>(rm, random, node);
	}
	if (type == module::flamethrower::type_name) {
		return std::make_unique<module::flamethrower>(rm, random, node);
	}
	if (type == module::healing_turret::type_name) {
		return std::make_unique<module::healing_turret>(rm, random, node);
	}
	if (type == module::shield_generator::type_name) {
		return std::make_unique<module::shield_generator>(rm, random, node);
	}
	if (type == module::turret::type_name) {
		return std::make_unique<module::turret>(rm, random, node);
	}
	if (type == module::warp_drive::type_name) {
		return std::make_unique<module::warp_drive>(rm, random, node);
	}
	return nullptr;
}
