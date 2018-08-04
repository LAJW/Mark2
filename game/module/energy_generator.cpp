#include "energy_generator.h"
#include <property_manager.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <update_context.h>
#include <world.h>

void mark::module::energy_generator::update(update_context& context)
{
	this->module::base::update(context);
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
	return "Energy Generator\n" + base::describe();
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
void mark::module::energy_generator::bind(
	prop_man& property_manager,
	T& instance)
{
	MARK_BIND(cur_energy);
	MARK_BIND(max_energy);
	MARK_BIND(energy_regen);
}

mark::module::energy_generator::energy_generator(
	resource::manager& rm,
	mark::random& random,
	const YAML::Node& node)
	: module::base(rm, random, node)
	, m_image_base(rm.image("energy-generator.png"))
	, m_image_bar(rm.image("bar.png"))
{
	property_manager property_manager(random);
	bind(property_manager, *this);
	if (property_manager.deserialize(node)) {
		throw std::runtime_error(
			"Could not deserialize " + std::string(type_name));
	}
}

void mark::module::energy_generator::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	property_serializer property_serializer;
	bind(property_serializer, *this);
	property_serializer.serialize(out);
	base::serialize(out);
	out << EndMap;
}

auto mark::module::energy_generator::passive() const noexcept -> bool
{
	return true;
}
