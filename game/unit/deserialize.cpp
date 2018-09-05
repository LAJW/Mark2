#include <stdafx.h>

#include "bucket.h"
#include "exception.h"
#include "gate.h"
#include "landing_pad.h"
#include "modular.h"
#include "projectile.h"
#include <algorithm.h>
#include <module/base.h>

auto mark::unit::deserialize(
	world& world,
	random& random,
	const YAML::Node& node)
	-> not_null<shared_ptr<unit::base>>
{
	let type = node["type"].as<std::string>();
	if (type == unit::gate::type_name) {
		return make_shared<unit::gate>(world, node);
	}
	if (type == unit::modular::type_name) {
		return make_shared<unit::modular>(ref(world), ref(random), node);
	}
	if (type == unit::landing_pad::type_name) {
		return make_shared<unit::landing_pad>(world, node);
	}
	if (type == unit::projectile::type_name) {
		return make_shared<unit::projectile>(world, random, node);
	}
	if (type == unit::bucket::type_name) {
		return make_shared<unit::bucket>(world, random, node);
	}
	throw exception("UNKNOWN_UNIT");
}
