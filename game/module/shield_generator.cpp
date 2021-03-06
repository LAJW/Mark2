﻿#include "shield_generator.h"
#include <property_manager.h>
#include <resource/image.h>
#include <resource/manager.h>
#include <sprite.h>
#include <sstream>
#include <stdafx.h>
#include <unit/modular.h>
#include <update_context.h>
#include <world.h>

let constexpr min_resistance = -2.f;
let constexpr max_resistance = .75f;

template <typename prop_man, typename T>
void mark::module::shield_generator::bind(
	prop_man& property_manager,
	T& instance)
{
	MARK_BIND(radius);
	MARK_BIND(cur_shield);
	MARK_BIND(max_shield);
	MARK_BIND(radius);
	MARK_BIND(shield_per_energy);
	MARK_BIND(broken);
	MARK_BIND(reboot_level);
	MARK_BIND(reflective);
}

auto mark::module::shield_generator::active() const -> bool
{
	return m_on && !m_stunned && !m_broken && m_cur_shield >= 0.f;
}

mark::module::shield_generator::shield_generator(
	resource::manager& rm,
	random& random,
	const YAML::Node& node)
	: module::base(rm, random, node)
	, m_im_generator(rm.image("shield-generator.png"))
	, m_model_shield(rm, random, node["radius"].as<float>(default_radius))
{
	property_manager property_manager(random);
	bind(property_manager, *this);
	if (property_manager.deserialize(node)) {
		throw std::runtime_error(
			"Could not deserialize " + std::string(type_name));
	}
}

void mark::module::shield_generator::serialize(YAML::Emitter& out) const
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

void mark::module::shield_generator::update(update_context& context)
{
	this->module::base::update(context);
	m_model_shield.update(context, this->pos(), this->active());
	// Recharge
	for (auto& module : this->neighbors()) {
		if (m_cur_shield >= m_max_shield) {
			break;
		}
		m_cur_shield = std::min(
			m_max_shield,
			m_cur_shield
				+ module.first.harvest_energy(context.dt)
					* m_shield_per_energy);
		if (m_cur_shield / m_max_shield >= m_reboot_level) {
			m_broken = false;
		}
	}
	this->render(context);
}

void mark::module::shield_generator::render(update_context& context) const
{
	let pos = this->pos();
	let module_size = module::size * 2.f;
	context.sprites[2].emplace_back([&] {
		sprite _;
		_.image = m_im_generator;
		_.pos = pos;
		_.size = module_size;
		_.rotation = parent().rotation();
		_.color = this->heat_color();
		return _;
	}());
	context.render([&] {
		update_context::bar_info _;
		_.image = parent().world().image_bar;
		_.pos = pos + vd(0, -module_size);
		_.type = update_context::bar_kind::shield;
		_.percentage = m_cur_shield / m_max_shield;
		return _;
	}());
}

auto mark::module::shield_generator::damage(
	const interface::damageable::info& attr) -> bool
{
	if (!this->active()) {
		return base::damage(attr);
	}
	if (attr.team == parent().team() || !attr.damaged->insert(this).second) {
		return false;
	}
	m_model_shield.trigger(attr.pos);
	let damage_result = base::damage_impl(attr);
	m_cur_shield = std::max(0.f, m_cur_shield - damage_result.health);
	if (m_cur_shield == 0.f) {
		m_broken = true;
	}
	parent().knockback(*attr.knocked, atan(pos() - attr.pos), attr.knockback);
	return true;
}

auto mark::module::shield_generator::describe() const -> std::string
{
	std::ostringstream os;
	os << "Shield Generator Module" << std::endl;
	os << "Shields: " << static_cast<int>(std::ceil(m_cur_shield)) << " of "
	   << static_cast<int>(std::ceil(m_max_shield)) << std::endl;
	os << "Radius: " << static_cast<int>(std::ceil(m_radius)) << std::endl;
	os << "Shield Per Energy: "
	   << static_cast<int>(std::ceil(m_shield_per_energy)) << std::endl;
	os << base::describe();
	return os.str();
}

auto mark::module::shield_generator::collide(const segment_t& ray)
	-> std::optional<std::pair<interface::damageable&, vd>>
{
	if (!this->active()) {
		return module::base::collide(ray);
	}
	if (let intersection = intersect(ray, pos(), m_radius)) {
		return { std::pair<interface::damageable&, vd>(*this, *intersection) };
	}
	return {};
}

auto mark::module::shield_generator::shield() const noexcept -> float
{
	return m_cur_shield;
}

auto mark::module::shield_generator::passive() const noexcept -> bool
{
	return false;
}

void mark::module::shield_generator::command(const command::any& any)
{
	if (std::holds_alternative<command::queue>(any)) {
		m_on = !m_on;
	}
}

auto mark::module::shield_generator::reflective() const -> bool
{
	return m_reflective && m_cur_shield >= 0.f;
}

auto mark::module::shield_generator::radius() const -> double
{
	return m_radius;
}
