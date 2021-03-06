﻿#include "battery.h"
#include <property_manager.h>
#include <resource/manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <update_context.h>

void mark::module::battery::update(update_context& context)
{
	this->module::base::update(context);
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_image_base;
		_.pos = this->pos();
		_.size = module::size * 2.f;
		_.rotation = parent_rotation();
		let frame = (1.f - m_cur_energy / m_max_energy) * 4.f;
		_.frame = static_cast<uint8_t>(std::round(frame));
		_.color = this->heat_color();
		return _;
	}());
	for (auto& module : this->neighbors()) {
		if (m_cur_energy < m_max_energy
			&& module.first.energy_ratio() > this->energy_ratio()) {
			m_cur_energy += module.first.harvest_energy(context.dt);
		}
	}
}

auto mark::module::battery::describe() const -> std::string
{
	return "Battery" + base::describe();
}

auto mark::module::battery::harvest_energy(double dt) -> float
{
	let delta = static_cast<float>(1.0 * dt);
	if (m_cur_energy > delta) {
		m_cur_energy -= delta;
		return delta;
	}
	return 0.f;
}

auto mark::module::battery::energy_ratio() const -> float
{
	return m_cur_energy / m_max_energy;
}

// deserialize / serialize

template <typename prop_man, typename T>
void mark::module::battery::bind(prop_man& property_manager, T& instance)
{
	MARK_BIND(max_energy);
	MARK_BIND(cur_energy);
}

mark::module::battery::battery(
	resource::manager& rm,
	random& random,
	const YAML::Node& node)
	: module::base(rm, random, node)
	, m_image_base(rm.image("battery.png"))
{
	property_manager property_manager(random);
	bind(property_manager, *this);
	if (property_manager.deserialize(node)) {
		throw std::runtime_error(
			"Could not deserialize " + std::string(type_name));
	}
}

void mark::module::battery::serialize(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	property_serializer property_serializer;
	bind(property_serializer, *this);
	base::serialize(out);
	property_serializer.serialize(out);
	out << EndMap;
}

auto mark::module::battery::passive() const noexcept -> bool { return true; }
