#include "stdafx.h"
#include "unit_gate.h"
#include "world.h"
#include "sprite.h"
#include "resource_manager.h"
#include "command.h"
#include "tick_context.h"
#include "unit_modular.h"

mark::unit::gate::gate(mark::world& world, mark::vector<double> pos):
	mark::unit::base(world, pos),
	m_image(world.resource_manager().image("kappa.png")) {
}

void mark::unit::gate::tick(mark::tick_context& context) {
	mark::sprite::info info;
	info.image = m_image;
	info.pos = m_pos;
	info.size = 320.f;
	info.rotation = 0.f;
	context.sprites[0].emplace_back(info);
}

void mark::unit::gate::activate(const std::shared_ptr<mark::unit::base>& by) {
	auto modular = std::dynamic_pointer_cast<mark::unit::modular>(by);
	if (modular) {
		m_world.next();
	}
}

// Serialize / Deserialize

mark::unit::gate::gate(
	mark::world& world,
	const YAML::Node& node):
	mark::unit::base(world, node),
	m_image(world.resource_manager().image("kappa.png")) {
}

void mark::unit::gate::serialize(YAML::Emitter& out) const {
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << mark::unit::gate::type_name;
	this->serialize_base(out);
	out << EndMap;
}

auto mark::unit::gate::collide(const mark::segment_t &) -> std::pair<mark::idamageable *, mark::vector<double>> {
	return { };
}
