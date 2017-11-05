#include "stdafx.h"
#include "module_core.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"

mark::module::core::core(resource::manager& rm, const YAML::Node& node):
	module::base(rm, node),
	m_image(rm.image("core.png")) { }

mark::module::core::core(resource::manager& resource_manager):
	base({ 2, 2 }, resource_manager.image("core.png")),
	m_image(resource_manager.image("core.png")) {
	
}

void mark::module::core::tick(tick_context& context) {
	this->module::base::tick(context);
	sprite info;
	info.image = m_image;
	info.pos = this->pos();
	info.size = 32.f;
	info.rotation = parent_rotation();
	info.color = this->heat_color();
	context.sprites[2].emplace_back(info);
}

auto mark::module::core::detachable() const -> bool {
	return false;
}

auto mark::module::core::describe() const ->std::string {
	return "Core Module";
}

void mark::module::core::serialise(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << EndMap;
}

auto mark::module::core::passive() const noexcept -> bool
{ return true; }
