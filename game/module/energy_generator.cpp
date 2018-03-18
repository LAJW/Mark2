#include <stdafx.h>
#include "energy_generator.h"
#include <resource_manager.h>
#include <sprite.h>
#include <tick_context.h>
#include <world.h>

mark::module::energy_generator::energy_generator(resource::manager& manager)
	: m_image_base(manager.image("energy-generator.png"))
	, m_image_bar(manager.image("bar.png"))
	, module::base({2, 2}, manager.image("energy-generator.png"))
{
}

void mark::module::energy_generator::tick(tick_context& context)
{
	this->module::base::tick(context);
	m_cur_energy = std::min(
		m_cur_energy + m_energy_regen * static_cast<float>(context.dt),
		m_max_energy);
	let pos = this->pos();
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image_base;
		_.pos = pos;
		_.size = module::size * 2.f;
		_.rotation = parent_rotation();
		_.frame =
			static_cast<uint8_t>(std::round(m_cur_energy / m_max_energy * 4.f));
		_.color = this->heat_color();
		return _;
	}());
}

auto mark::module::energy_generator::describe() const -> std::string
{
	return "Energy Generator";
}

auto mark::module::energy_generator::harvest_energy(double dt) -> float
{
	let delta = static_cast<float>(1.0 * dt);
	if (delta <= m_cur_energy) {
		m_cur_energy -= delta;
		return delta;
	}
	return 0.f;
}

auto mark::module::energy_generator::energy_ratio() const -> float
{
	return m_cur_energy / m_max_energy;
}

// Serialize / Deserialize

mark::module::energy_generator::energy_generator(
	resource::manager& rm,
	const YAML::Node& node)
	: module::base(rm, node)
	, m_image_base(rm.image("energy-generator.png"))
	, m_image_bar(rm.image("bar.png"))
	, m_cur_energy(node["cur_energy"].as<float>())
	, m_max_energy(node["max_energy"].as<float>())
	, m_energy_regen(node["energy_regen"].as<float>())
{
}

void mark::module::energy_generator::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << Key << "max_energy" << Value << m_max_energy;
	out << Key << "cur_energy" << Value << m_cur_energy;
	out << Key << "energy_regen" << Value << m_energy_regen;
	out << EndMap;
}

auto mark::module::energy_generator::passive() const noexcept -> bool
{
	return true;
}
