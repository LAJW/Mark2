#include <stdafx.h>
#include "bucket.h"
#include "exception.h"
#include "gate.h"
#include "landing_pad.h"
#include "modular.h"
#include "projectile.h"
#include <module/base.h>

auto mark::unit::deserialize(world& world, const YAML::Node& node)
	-> shared_ptr<unit::base>
{

	let type = node["type"].as<std::string>();
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
