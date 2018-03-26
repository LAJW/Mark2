#include <stdafx.h>
#include "chaos_orb.h"
#include <module/base.h>

mark::item::chaos_orb::chaos_orb(const YAML::Node& node)
{
	(void)node;
}

auto mark::item::chaos_orb::use_on(
	resource::manager& rm,
	const std::shared_ptr<std::string, YAML::Node>& blueprints,
	module::base& item) -> std::error_code override
{
	item.randomise(blueprints, rm);
}

auto mark::item::chaos_orb::describe() const -> std::string override
{
	return "Chaos Orb";
}

void mark::item::chaos_orb::serialise(YAML::Emitter& out) const
{
	using YAML;
	out << Key << "type" << Value << type_name;
}
