#include "exception.h"
#include "module_base.h"
#include "unit_bucket.h"
#include "unit_modular.h"
#include "unit_projectile.h"
#include "unit_landing_pad.h"

auto mark::unit::deserialize(
	mark::world& world,
	const YAML::Node& node) ->
	std::shared_ptr<mark::unit::base> {

	const auto type = node["type"].as<std::string>();
	if (type == mark::unit::modular::type_name) {
		return std::make_shared<mark::unit::modular>(world, node);
	} else if (type == mark::unit::landing_pad::type_name) {
		return std::make_shared<mark::unit::landing_pad>(world, node);
	} else if (type == mark::unit::projectile::type_name) {
		return std::make_shared<mark::unit::projectile>(world, node);
	} else if (type == mark::unit::bucket::type_name) {
		return std::make_shared<mark::unit::bucket>(world, node);
	} else {
		throw mark::exception("UNKNOWN_UNIT");
	}
}

