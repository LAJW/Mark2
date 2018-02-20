#include "stdafx.h"
#include "module_base_turret.h"
#include "unit_modular.h"
#include "world.h"
#include "resource_manager.h"
#include "interface_world_object.h"

mark::module::base_turret::base_turret(
	const vector<unsigned> size,
	const std::shared_ptr<const resource::image>& image)
	: module::base(size, image)
	, m_targeting_system(*this)
{ }

mark::module::base_turret::base_turret(
	resource::manager & rm,
	const YAML::Node & node)
	: module::base(rm, node)
	, m_targeting_system(*this)
{ }

void mark::module::base_turret::tick()
{
	m_targeting_system.tick();
}

auto mark::module::base_turret::request_charge() -> bool
{
	return !m_stunned && m_targeting_system.request_charge();
}

auto mark::module::base_turret::target() const -> std::optional<vector<double>>
{
	return m_targeting_system.target();
}

void mark::module::base_turret::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	base::serialise(out);
}

auto mark::module::base_turret::passive() const noexcept -> bool
{ return false; }

void mark::module::base_turret::command(const command::any& any)
{
	m_targeting_system.command(any);
}

