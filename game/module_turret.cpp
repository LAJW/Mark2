#pragma once
#include "module_turret.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_projectile.h"
#include "unit_modular.h"

mark::module::turret::turret(mark::resource::manager& resource_manager):
	base({ 2, 2 }, resource_manager.image("turret-base.png")),
	m_im_base(resource_manager.image("turret-base.png")),
	m_im_cannon(resource_manager.image("turret-cannon.png")),
	m_adsr(0.1f, 8.f, 0.1f, 0.8f) {

}

void mark::module::turret::tick(mark::tick_context& context) {
	m_adsr.tick(context.dt);
	auto pos = socket()->relative_pos();
	if (m_cur_cooldown >= 0) {
		m_cur_cooldown -= static_cast<float>(context.dt);
	} else {
		m_cur_cooldown = 0.5f;
		m_adsr.trigger();
		auto projectile = std::make_shared<mark::unit::projectile>(socket()->world(), pos, socket()->rotation());
		projectile->team(socket()->team());
		context.units.emplace_back(std::move(projectile));
	}
	context.sprites[1].push_back(mark::sprite(m_im_cannon, pos - mark::rotate(mark::vector<double>(m_adsr.get() - 16.f, 0.0), socket()->rotation()), 32.f, socket()->rotation()));
	context.sprites[0].push_back(mark::sprite(m_im_base, pos.x, pos.y, 32.f, socket()->rotation()));

}

auto mark::module::turret::dead() const -> bool {
	return false;
}