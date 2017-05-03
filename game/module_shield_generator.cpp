#pragma once
#include "module_shield_generator.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"
#include "world.h"
#include <sstream>

mark::module::shield_generator::shield_generator(mark::resource::manager& resource_manager):
	base({ 2, 2 }, resource_manager.image("shield-generator.png")),
	m_im_generator(resource_manager.image("shield-generator.png")),
	m_model_shield(resource_manager, 128.f) {

}

void mark::module::shield_generator::tick(mark::tick_context& context) {
	const auto pos = this->pos();
	if (m_cur_shield > 0) {
		m_model_shield.tick(context, pos);
	}
	context.sprites[0].push_back(mark::sprite(m_im_generator, pos, mark::module::size * 2.f, parent().rotation()));
	context.render_bar(
		parent().world().resource_manager().image("bar.png"),
		pos + mark::vector<double>(0, -mark::module::size * 2),
		mark::tick_context::bar_type::shield,
		m_cur_shield / m_max_shield);
	context.render_bar(
		parent().world().resource_manager().image("bar.png"),
		pos + mark::vector<double>(0, -mark::module::size * 2 - 8.f),
		mark::tick_context::bar_type::health,
		m_cur_health / m_max_health);
}

auto mark::module::shield_generator::collides(mark::vector<double> pos, float radius) const -> bool {
	return mark::length(this->pos() - pos) < radius + 64.f;
}

bool mark::module::shield_generator::damage(const mark::idamageable::attributes& attr) {
	if (m_cur_shield > 0.f && attr.team != parent().team()) {
		m_model_shield.trigger(attr.pos);
		m_cur_shield -= attr.physical;
		return true;
	} else if (m_cur_shield <= 0.f && attr.team != parent().team()) {
		m_cur_health -= attr.physical;
		return true;
	}
	return false;
}

auto mark::module::shield_generator::describe() const -> std::string {
	std::ostringstream os;
	os << "Shield Generator Module" << std::endl;
	os << "Health: " << static_cast<int>(std::ceil(m_cur_health)) << " of " << static_cast<int>(std::ceil(m_max_health)) << std::endl;
	os << "Shields: " << static_cast<int>(std::ceil(m_cur_shield)) << " of " << static_cast<int>(std::ceil(m_max_shield)) << std::endl;
	return os.str();
}

auto mark::module::shield_generator::collide(const mark::segment_t& ray) ->
	std::pair<mark::idamageable*, mark::vector<double>> {
	if (m_cur_shield >= 0.f) {
		const auto shield_size = 64.f;
		const auto intersection = mark::intersect(ray, pos(), shield_size);
		if (!std::isnan(intersection.x)) {
			return { this, intersection };
		} else {
			return { nullptr, { NAN, NAN } };
		}
	} else {
		return mark::module::base::collide(ray);
	}
}
