#pragma once
#include "stdafx.h"

namespace mark {

class world_stack final
{
public:
	using blueprint_map = std::unordered_map<std::string, YAML::Node>;
	world_stack(
		const YAML::Node& state_node,
		resource::manager& resource_manager,
		const blueprint_map& blueprints);
	void next();
	void prev();
	auto world() noexcept -> mark::world&;
	auto blueprints() const -> const blueprint_map&;
	~world_stack();

private:
	struct world_data
	{
		unique_ptr<mark::world> world;
		vd target_pos;
	};
	std::vector<world_data> m_worlds;
	size_t m_current_world_id = 0;
	resource::manager& m_resource_manager;
	const blueprint_map m_blueprints;
};
} // namespace mark
