#include "stdafx.h"
#include "module_engine.h"
#include "resource_manager.h"
#include "tick_context.h"

mark::module::engine::engine(resource::manager& manager)
	: m_image_base(manager.image("engine.png"))
	, module::base({ 4, 2 }, manager.image("engine.png")) { }

void mark::module::engine::tick(tick_context& context) {
	this->module::base::tick(context);
	const auto pos = this->pos();

	sprite info;
	info.image = m_image_base;
	info.pos = pos;
	info.size = module::size * 4.f;
	info.rotation = parent_rotation();
	info.color = this->heat_color();
	context.sprites[2].emplace_back(info);
}

auto mark::module::engine::describe() const->std::string {
	return "Engine";
}

auto mark::module::engine::global_modifiers() const->module::modifiers {
	module::modifiers mods;
	if (m_active) {
		mods.velocity = 150.f;
	}
	return mods;
}

void mark::module::engine::command(const command::any& any)
{
	if (std::holds_alternative<command::use>(any)) {
		m_active = true;
	} else if (std::holds_alternative<command::release>(any)) {
		m_active = false;
	}
}

mark::module::engine::engine(resource::manager& rm, const YAML::Node& node):
	module::base(rm, node),
	m_image_base(rm.image("engine.png")) { }

void mark::module::engine::serialise(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << EndMap;
}

auto mark::module::engine::reserved() const noexcept -> reserved_type
{ return reserved_type::back; }

auto mark::module::engine::passive() const noexcept -> bool
{ return false; }

