#include "stdafx.h"
#include "module_warp_drive.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_modular.h"
#include "world.h"
#include "map.h"

mark::module::warp_drive::warp_drive(resource::manager& manager)
	: m_image_base(manager.image("engine.png"))
	, module::base({ 4, 2 }, manager.image("engine.png")) { }

void mark::module::warp_drive::tick(tick_context& context) {
	this->module::base::tick(context);
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image_base;
		_.pos = this->pos();
		_.size = module::size * 4.f;
		_.rotation = parent_rotation();
		_.color = this->heat_color();
		return _;
	}());
}

auto mark::module::warp_drive::describe() const->std::string {
	return "Warp Drive";
}

void mark::module::warp_drive::command(const command::any& any)
{
	if (let activate = std::get_if<command::activate>(&any)) {
		if (this->m_stunned || m_cur_heat > 5.f) {
			return;
		}
		let path = world().map().find_path(
			parent().pos(),
			activate->pos,
			parent().radius());
		if (!path.empty()) {
			parent().pos(path.front());
		}
		m_cur_heat = max_heat;
	}
}

mark::module::warp_drive::warp_drive(resource::manager& rm, const YAML::Node& node):
	module::base(rm, node),
	m_image_base(rm.image("engine.png")) { }

void mark::module::warp_drive::serialise(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << EndMap;
}

auto mark::module::warp_drive::passive() const noexcept -> bool
{ return false; }
