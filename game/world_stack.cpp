#include "stdafx.h"
#include "world.h"
#include "world_stack.h"
#include "exception.h"

mark::world_stack::world_stack(
	resource::manager& resource_manager,
	const std::unordered_map<std::string, YAML::Node>& templates)
	: m_templates(templates)
	, m_resource_manager(resource_manager)
{
	m_worlds.emplace_back(
		std::make_unique<mark::world>(resource_manager, templates));
}

void mark::world_stack::next()
{
	++m_current_world_id;
	if (m_current_world_id == m_worlds.size()) {
		m_worlds.emplace_back(
			std::make_unique<mark::world>(m_resource_manager, m_templates));
	}
	world().attach(m_worlds[m_current_world_id - 1]->target());
}

void mark::world_stack::prev()
{
	if (m_current_world_id == 0)
		throw user_error("NO_PREV_WORLD_FOUND");
	--m_current_world_id;
	world().attach(m_worlds[m_current_world_id + 1]->target());
}

auto mark::world_stack::world() noexcept -> mark::world& 
{ return *m_worlds[m_current_world_id]; }

mark::world_stack::~world_stack() = default;
