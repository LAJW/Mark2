#include <stdafx.h>
#include "chaos_orb.h"
#include <module/base.h>
#include <resource_manager.h>
#include <exception.h>

mark::item::chaos_orb::chaos_orb(
	mark::resource::manager& rm,
	const YAML::Node& node) : m_thumbnail(rm.image("chaos-orb.png"))
{
	// TODO stacks
	(void)node;
}

auto mark::item::chaos_orb::use_on(
	resource::manager& rm,
	const std::unordered_map<std::string, YAML::Node>& blueprints,
	module::base& item) -> std::error_code
{
	item.randomise(blueprints, rm);
	return error::code::success;
}

auto mark::item::chaos_orb::describe() const -> std::string
{
	return "Chaos Orb";
}

void mark::item::chaos_orb::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << Key << "type" << Value << type_name;
}

auto mark::item::chaos_orb::size() const -> vector<unsigned>
{
	return { 2, 2 };
}

auto mark::item::chaos_orb::thumbnail() const -> std::shared_ptr<const resource::image>
{
	return m_thumbnail;
}
