#include "stdafx.h"
#include "world.h"
#include "world_stack.h"
#include "exception.h"
#include "unit_base.h"

mark::world_stack::world_stack(
	const YAML::Node& state_node,
	resource::manager& resource_manager,
	const std::unordered_map<std::string, YAML::Node>& templates)
	: m_templates(templates)
	, m_resource_manager(resource_manager)
{
	world_data data;
	data.world = std::make_unique<mark::world>(
		*this, resource_manager, state_node, m_templates);
	m_worlds.push_back(std::move(data));
}

void mark::world_stack::next()
{
	// Can't use always - pointer gets invalidated on vector push
	auto& old_data = m_worlds[m_current_world_id];
	auto target = old_data.world->target();
	old_data.target_pos = target->pos();
	++m_current_world_id;
	if (m_current_world_id == m_worlds.size()) {
		world_data data;
		data.world = std::make_unique<mark::world>(
			*this, m_resource_manager, m_templates, false, false);
		m_worlds.push_back(std::move(data));
		target->pos({ 0., 0. });
	} else {
		target->pos(m_worlds[m_current_world_id].target_pos);
	}
	world().attach(target);
	world().target(target);
}

void mark::world_stack::prev()
{
	if (m_current_world_id == 0)
		throw user_error("NO_PREV_WORLD_FOUND");
	auto& old_data = m_worlds[m_current_world_id];
	old_data.target_pos = old_data.world->target()->pos();
	--m_current_world_id;
	old_data.world->target()->pos(m_worlds[m_current_world_id].target_pos);
	world().attach(old_data.world->target());
}

auto mark::world_stack::world() noexcept -> mark::world& 
{ return *m_worlds[m_current_world_id].world; }

mark::world_stack::~world_stack() = default;
