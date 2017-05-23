#include "terrain_base.h"
#include "exception.h"
#include "terrain_floor.h"
#include "terrain_wall.h"

auto mark::terrain::base::deserialize(mark::resource::manager& resource_manager, const YAML::Node& node) ->
	std::shared_ptr<mark::terrain::base> {
	const auto type = node["type"].as<std::string>();
	if (type == "terrain_floor") {
		return std::make_shared<mark::terrain::floor>(resource_manager, node);
	} else if (type == "terrain_wall") {
		return std::make_shared<mark::terrain::wall>(resource_manager);
	} else {
		throw mark::exception("UNKNOWN_TERRAIN");
	}
}
