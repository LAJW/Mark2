#pragma once
#include "stdafx.h"

namespace mark {
namespace resource {
class manager;
}
class world;
class world_stack final {
public:
	world_stack(
		const YAML::Node& state_node,
		resource::manager& resource_manager,
		const std::unordered_map<std::string, YAML::Node>& templates);
	void next();
	void prev();
	auto world() noexcept -> mark::world&;
	auto templates() const -> const std::unordered_map<std::string, YAML::Node>&;
	~world_stack();
private:
	struct world_data {
		std::unique_ptr<mark::world> world;
		vector<double> target_pos;
	};
	std::vector<world_data> m_worlds;
	size_t m_current_world_id = 0;
	resource::manager& m_resource_manager;
	const std::unordered_map<std::string, YAML::Node>& m_templates;
};
}