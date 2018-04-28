﻿#include <stdafx.h>
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
#include <item/chaos_orb.h>

auto mark::module::deserialize(resource::manager& rm, const YAML::Node& node)
	-> interface::item_ptr
{
	let type = node["type"].as<std::string>();
	if (type == module::battery::type_name) {
		return std::make_unique<module::battery>(rm, node);
	} else if (type == module::laser::type_name) {
		return std::make_unique<module::laser>(rm, node);
	} else if (type == module::cargo::type_name) {
		return std::make_unique<module::cargo>(rm, node);
	} else if (type == module::core::type_name) {
		return std::make_unique<module::core>(rm, node);
	} else if (type == module::energy_generator::type_name) {
		return std::make_unique<module::energy_generator>(rm, node);
	} else if (type == module::engine::type_name) {
		return std::make_unique<module::engine>(rm, node);
	} else if (type == module::flamethrower::type_name) {
		return std::make_unique<module::flamethrower>(rm, node);
	} else if (type == module::healing_turret::type_name) {
		return std::make_unique<module::healing_turret>(rm, node);
	} else if (type == module::shield_generator::type_name) {
		return std::make_unique<module::shield_generator>(rm, node);
	} else if (type == module::turret::type_name) {
		return std::make_unique<module::turret>(rm, node);
	} else if (type == module::warp_drive::type_name) {
		return std::make_unique<module::warp_drive>(rm, node);
	} else if (type == item::chaos_orb::type_name) {
		return std::make_unique<item::chaos_orb>(rm, node);
	} else {
		throw exception("UNKNOWN_MODULE");
	}
}
