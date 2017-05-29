#include "exception.h"
#include "unit_modular.h"
#include "unit_projectile.h"

auto mark::unit::base::deserialize(mark::world& world, const YAML::Node& node) ->
	std::shared_ptr<mark::unit::base> {
	const auto type = node["type"].as<std::string>();
	if (type == "modular") {
		return std::make_shared<mark::unit::modular>(world, node);
	} else {
		throw mark::exception("UNKNOWN_UNIT");
	}
}

