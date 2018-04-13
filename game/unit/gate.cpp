#include <stdafx.h>
#include "gate.h"
#include "modular.h"
#include <command.h>
#include <model_animated.h>
#include <resource_manager.h>
#include <sprite.h>
#include <update_context.h>
#include <world.h>

constexpr let UNIT_GATE_SIZE = 256.f;

mark::unit::gate::gate(const info& info)
	: activable(info)
	, m_im_base(info.world->resource_manager().image("gate-base.png"))
	, m_rotor(info.world->resource_manager().image("gate-rotor.png"))
	, m_im_enclosure(info.world->resource_manager().image("gate-enclosure.png"))
	, m_inverted(info.inverted)
{}

void mark::unit::gate::update(update_context& context)
{
	let target = world().target();
	let angle = target ? static_cast<float>(atan(pos() - target->pos())) : 0.f;
	sprite info;
	info.image = m_im_base;
	info.pos = pos();
	info.size = UNIT_GATE_SIZE;
	info.rotation = 0.f;
	context.sprites[0].emplace_back(info);

	m_rotor.update(context.dt);
	context.sprites[10].emplace_back(
		m_rotor.render(pos(), UNIT_GATE_SIZE, angle, sf::Color::White));

	sprite enclosure_info;
	enclosure_info.image = m_im_enclosure;
	enclosure_info.pos = pos();
	enclosure_info.size = UNIT_GATE_SIZE;
	enclosure_info.rotation = angle;
	context.sprites[10].emplace_back(enclosure_info);
}

auto mark::unit::gate::use(const std::shared_ptr<unit::modular>&)
	-> std::error_code
{
	if (m_inverted) {
		world().prev();
	} else {
		world().next();
	}
	return error::code::success;
}

// Serialize / Deserialize

mark::unit::gate::gate(mark::world& world, const YAML::Node& node)
	: activable(world, node)
	, m_im_base(world.resource_manager().image("gate-base.png"))
	, m_rotor(world.resource_manager().image("gate-rotor.png"))
	, m_im_enclosure(world.resource_manager().image("gate-enclosure.png"))
	, m_inverted(node["inverted"].as<bool>(false))
{}

void mark::unit::gate::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << unit::gate::type_name;
	out << Key << "inverted" << Value << m_inverted;
	base::serialize(out);
	out << EndMap;
}
