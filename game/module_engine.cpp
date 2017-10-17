#include "stdafx.h"
#include "module_engine.h"
#include "resource_manager.h"
#include "tick_context.h"

mark::module::engine::engine(mark::resource::manager& manager)
	: m_image_base(manager.image("engine.png"))
	, mark::module::base({ 4, 2 }, manager.image("engine.png")) { }

void mark::module::engine::tick(mark::tick_context& context) {
	this->mark::module::base::tick(context);
	const auto pos = this->pos();

	mark::sprite info;
	info.image = m_image_base;
	info.pos = pos;
	info.size = mark::module::size * 4.f;
	info.rotation = parent().rotation();
	info.color = this->heat_color();
	context.sprites[2].emplace_back(info);
}

auto mark::module::engine::describe() const->std::string {
	return "Engine";
}

auto mark::module::engine::global_modifiers() const->mark::module::modifiers {
	mark::module::modifiers mods;
	if (m_active) {
		mods.velocity = 150.f;
	}
	return mods;
}

void mark::module::engine::shoot(mark::vector<double>, bool release) {
	m_active = !release;
}

mark::module::engine::engine(mark::resource::manager& rm, const YAML::Node& node):
	mark::module::base(rm, node),
	m_image_base(rm.image("engine.png")) { }

void mark::module::engine::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	this->serialize_base(out);
	out << EndMap;
}

auto mark::module::engine::reserved() const noexcept -> reserved_type
{ return reserved_type::back; }

auto mark::module::engine::passive() const noexcept -> bool
{ return false; }

