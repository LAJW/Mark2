#include "stdafx.h"
#include "exception.h"
#include "module_base.h"
#include "unit_bucket.h"
#include "unit_modular.h"
#include "unit_projectile.h"
#include "unit_landing_pad.h"
#include "unit_gate.h"

auto mark::unit::deserialise(
	world& world,
	const YAML::Node& node) ->
	std::shared_ptr<unit::base> {

	const auto type = node["type"].as<std::string>();
	if (type == unit::gate::type_name) {
		return std::make_shared<unit::gate>(world, node);
	} else if (type == unit::modular::type_name) {
		return std::make_shared<unit::modular>(world, node);
	} else if (type == unit::landing_pad::type_name) {
		return std::make_shared<unit::landing_pad>(world, node);
	} else if (type == unit::projectile::type_name) {
		return std::make_shared<unit::projectile>(world, node);
	} else if (type == unit::bucket::type_name) {
		return std::make_shared<unit::bucket>(world, node);
	} else {
		throw exception("UNKNOWN_UNIT");
	}
}

