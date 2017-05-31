#include "unit_base.h"
#include "exception.h"
#include "unit_modular.h"
#include "unit_projectile.h"
#include "module_base.h"
#include "unit_landing_pad.h"

auto mark::unit::deserialize(
	mark::world& world,
	const YAML::Node& node) ->
	std::shared_ptr<mark::unit::base> {

	const auto type = node["type"].as<std::string>();
	if (type == "modular") {
		return std::make_shared<mark::unit::modular>(world, node);
	} else if (type == "unit_landing_pad") {
		return std::make_shared<mark::unit::landing_pad>(world, node);
	} else if (type == "unit_projectile") {
		return std::make_shared<mark::unit::projectile>(world, node);
	} else {
		throw mark::exception("UNKNOWN_UNIT");
	}
}

