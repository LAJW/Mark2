﻿#include "stdafx.h"
#include <sstream>
#include "module_shield_generator.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_modular.h"
#include "world.h"

mark::module::shield_generator::shield_generator(
	resource::manager& rm, const YAML::Node& node)
	: module::base(rm, node)
	, m_im_generator(rm.image("shield-generator.png"))
	, m_model_shield(rm, 512.f) { }

mark::module::shield_generator::shield_generator(
	resource::manager& resource_manager)
	: base({ 2, 2 }, resource_manager.image("shield-generator.png"))
	, m_im_generator(resource_manager.image("shield-generator.png"))
	, m_model_shield(resource_manager, 512.f) { }

void mark::module::shield_generator::tick(tick_context& context)
{
	this->module::base::tick(context);
	let pos = this->pos();
	if (m_cur_shield > 0) {
		m_model_shield.tick(context, pos);
	}
	// Recharge
	let constexpr recharge_efficiency = 10.f;
	for (auto& module : this->neighbours()) {
		if (m_cur_shield < m_max_shield) {
			m_cur_shield = std::min(
				m_max_shield,
				m_cur_shield
				+ module.first.get().harvest_energy(context.dt)
					* recharge_efficiency);
		}
	}
	sprite info;
	info.image = m_im_generator;
	info.pos = pos;
	info.size = module::size * 2.f;
	info.rotation = parent().rotation();
	info.color = this->heat_color();
	context.sprites[2].emplace_back(info);

	tick_context::bar_info shield_bar;
	shield_bar.image = parent().world().resource_manager().image("bar.png");
	shield_bar.pos = pos + vector<double>(0, -module::size * 2);
	shield_bar.type = tick_context::bar_type::shield;
	shield_bar.percentage = m_cur_shield / m_max_shield;
	context.render(shield_bar);
}

auto mark::module::shield_generator::damage(
	const interface::damageable::info& attr) -> bool
{
	if (attr.team == parent().team()
		|| attr.damaged->find(this) != attr.damaged->end()) {
		return false;
	}
	attr.damaged->insert(this);
	if (m_cur_shield > 0.f) {
		m_model_shield.trigger(attr.pos);
		m_cur_shield -= attr.physical;
	} else {
		m_cur_health -= attr.physical;
	}
	return true;
}

auto mark::module::shield_generator::describe() const -> std::string
{
	std::ostringstream os;
	os << "Shield Generator Module" << std::endl;
	os << "Health: " << static_cast<int>(std::ceil(m_cur_health))
		<< " of " << static_cast<int>(std::ceil(m_max_health)) << std::endl;
	os << "Shields: " << static_cast<int>(std::ceil(m_cur_shield))
		<< " of " << static_cast<int>(std::ceil(m_max_shield)) << std::endl;
	return os.str();
}

auto mark::module::shield_generator::collide(const segment_t& ray) ->
	std::optional<std::pair<
		std::reference_wrapper<interface::damageable>,
		vector<double>>>
{
	if (m_cur_shield > 0.f) {
		let shield_size = 256.f;
		if (let intersection = intersect(ray, pos(), shield_size)) {
			return { {
				std::ref(static_cast<interface::damageable&>(*this)),
				*intersection } };
		}
		return { };
	}
	return module::base::collide(ray);
}

auto mark::module::shield_generator::shield() const noexcept -> float
{ return m_cur_shield; }

void mark::module::shield_generator::serialise(YAML::Emitter& out) const
{
	using namespace YAML;
	out << BeginMap;
	out << Key << "type" << Value << type_name;
	base::serialise(out);
	out << Key << "cur_shield" << Value << m_cur_shield;
	out << Key << "max_shield" << Value << m_max_shield;
	out << EndMap;
}

auto mark::module::shield_generator::passive() const noexcept -> bool
{ return true; }
