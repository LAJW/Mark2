#pragma once
#include "module_mortar.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_heat_seeker.h"
#include "unit_modular.h"

mark::module::mortar::mortar(mark::resource::manager& resource_manager) :
	base({ 2, 2 }, resource_manager.image("turret-base.png")),
	m_im_base(resource_manager.image("turret-base.png")),
	m_im_cannon(resource_manager.image("turret-cannon.png")),
	m_adsr(0.1f, 8.f, 0.1f, 0.8f) {

}

void mark::module::mortar::tick(mark::tick_context& context) {
	m_adsr.tick(context.dt);
	auto pos = socket()->relative_pos();
	if (m_cur_cooldown >= 0) {
		m_cur_cooldown -= static_cast<float>(context.dt);
	} else if (m_shoot) {
		const auto direction = static_cast<float>(-mark::sgn(socket()->pos().y + 1) * 90);
		m_cur_cooldown = 2.5f;
		m_adsr.trigger();
		const auto total_projectiles = 4;
		for (int i = 0; i < total_projectiles; i++) {
			float cur = static_cast<float>(i) / static_cast<float>(total_projectiles) * 30.f - 15.f - direction;
			auto projectile = std::make_shared<mark::unit::heat_seeker>(socket()->world(), pos, socket()->rotation() + cur);
			projectile->team(socket()->team());
			context.units.emplace_back(std::move(projectile));
		}
	}
	m_shoot = false;
	context.sprites[0].push_back(mark::sprite(m_im_base, pos.x, pos.y, 32.f, socket()->rotation()));

}

auto mark::module::mortar::dead() const -> bool {
	return false;
}

void mark::module::mortar::shoot(mark::vector<double> pos) {
	m_shoot = true;
}

auto mark::module::mortar::describe() const -> std::string {
	return "Mortar Module";
}
