#include "stdafx.h"
#include "world_stack.h"
#include "exception.h"
#include "unit/modular.h"
#include "world.h"

mark::world_stack::world_stack(
	const YAML::Node& state_node,
	resource::manager& resource_manager,
	const std::unordered_map<std::string, YAML::Node>& templates)
	: m_templates(templates)
	, m_resource_manager(resource_manager)
{
	world_data data;
	data.world =
		std::make_unique<mark::world>(*this, resource_manager, state_node);
	m_worlds.push_back(std::move(data));
}

void mark::world_stack::next()
{
	// Can't use always - pointer gets invalidated on vector push
	auto& old_data = m_worlds[m_current_world_id];
	auto target =
		std::dynamic_pointer_cast<unit::modular>(old_data.world->target());
	old_data.target_pos = target->pos();
	++m_current_world_id;
	if (m_current_world_id == m_worlds.size()) {
		world_data data;
		data.world =
			std::make_unique<mark::world>(*this, m_resource_manager, false);
		m_worlds.push_back(std::move(data));
		target->pos({0., 0.});
		target->stop();
	}
	else {
		target->pos(m_worlds[m_current_world_id].target_pos);
		target->stop();
	}
	world().attach(target);
	world().target(target);
}

void mark::world_stack::prev()
{
	if (m_current_world_id == 0)
		throw user_error("NO_PREV_WORLD_FOUND");
	let target = std::dynamic_pointer_cast<unit::modular>(world().target());
	auto& old_data = m_worlds[m_current_world_id];
	old_data.target_pos = target->pos();
	--m_current_world_id;
	target->pos(m_worlds[m_current_world_id].target_pos);
	target->stop();
	world().attach(target);
}

auto mark::world_stack::world() noexcept -> mark::world&
{
	return *m_worlds[m_current_world_id].world;
}

auto mark::world_stack::templates() const
	-> const std::unordered_map<std::string, YAML::Node>&
{
	return m_templates;
}

mark::world_stack::~world_stack() = default;
