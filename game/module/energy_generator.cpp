#include <stdafx.h>
#include "energy_generator.h"
#include <resource_manager.h>
#include <sprite.h>
#include <tick_context.h>
#include <world.h>
#include <property_manager.h>

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

template <typename prop_man, typename T>
void mark::module::energy_generator::bind(prop_man& property_manager, T& instance)
{
	MARK_BIND(cur_energy);
	MARK_BIND(max_energy);
	MARK_BIND(energy_regen);
}

void mark::module::energy_generator::bind(mark::property_manager& property_manager)
{
	bind(property_manager, *this);
	base::bind(property_manager);
}

mark::module::energy_generator::energy_generator(
	resource::manager& rm,
	const YAML::Node& node)
	: module::base(rm, node)
	, m_image_base(rm.image("energy-generator.png"))
	, m_image_bar(rm.image("bar.png"))
{
	property_manager property_manager(rm);
	bind(property_manager);
	if (property_manager.deserialise(node)) {
		throw std::runtime_error(
			"Could not deserialise " + std::string(type_name));
	}
}

void mark::module::energy_generator::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	property_serialiser property_serialiser;
	bind(property_serialiser, *this);
	property_serialiser.serialise(out);
	out << EndMap;
}

auto mark::module::energy_generator::passive() const noexcept -> bool
{
	return true;
}
