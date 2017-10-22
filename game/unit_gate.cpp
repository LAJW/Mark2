#include "stdafx.h"
#include "unit_gate.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "command.h"
#include "tick_context.h"
#include "unit_modular.h"

mark::unit::gate::gate(mark::world& world, vector<double> pos, bool inverted)
	: unit::base(world, pos)
	, m_image(world.resource_manager().image("kappa.png"))
	, m_inverted(inverted)
{ }

void mark::unit::gate::tick(tick_context& context) {
	sprite info;
	info.image = m_image;
	info.pos = pos();
	info.size = 320.f;
	info.rotation = 0.f;
	context.sprites[0].emplace_back(info);
}

void mark::unit::gate::activate(const std::shared_ptr<unit::base>& by) {
	if (auto modular = std::dynamic_pointer_cast<unit::modular>(by)) {
		if (m_inverted) {
			world().prev();
		} else {
			world().next();
		}
	}
}

// Serialize / Deserialize

mark::unit::gate::gate(
	mark::world& world,
	const YAML::Node& node)
	: unit::base(world, node)
	, m_image(world.resource_manager().image("kappa.png"))
	, m_inverted(node["inverted"].as<bool>(false)) { }

void mark::unit::gate::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << unit::gate::type_name;
	out << Key << "inverted" << Value << m_inverted;
	this->serialize_base(out);
	out << EndMap;
}
