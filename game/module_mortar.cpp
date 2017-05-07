#pragma once
#include "module_mortar.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_projectile.h"
#include "unit_modular.h"
#include "world.h"

mark::module::mortar::mortar(mark::resource::manager& resource_manager) :
	base({ 2, 2 }, resource_manager.image("turret-base.png")),
	m_im_base(resource_manager.image("turret-base.png")),
	m_im_cannon(resource_manager.image("turret-cannon.png")),
	m_adsr(0.1f, 8.f, 0.1f, 0.8f) {

}

void mark::module::mortar::tick(mark::tick_context& context) {
	m_adsr.tick(context.dt);
	auto pos = this->pos();
	if (m_cur_cooldown >= 0) {
		m_cur_cooldown -= static_cast<float>(context.dt);
	} else if (m_shoot) {
		const auto direction = static_cast<float>(-mark::sgn(grid_pos().y + 1) * 90);
		m_cur_cooldown = 2.5f;
		m_adsr.trigger();
		const auto total_projectiles = 4;
		for (int i = 0; i < total_projectiles; i++) {
			float cur = static_cast<float>(i) / static_cast<float>(total_projectiles) * 30.f - 15.f - direction;
			mark::unit::projectile::attributes attr;
			attr.world = &parent().world();
			attr.pos = pos;
			attr.rotation = parent().rotation() + cur;
			attr.velocity = 500.f;
			attr.seek_radius = 1000.f;
			attr.team = parent().team();
			context.units.emplace_back(std::make_shared<mark::unit::projectile>(attr));
		}
	}
	m_shoot = false;
	context.sprites[0].push_back(mark::sprite(m_im_base, pos, 32.f, parent().rotation()));

	mark::tick_context::bar_info bar;
	bar.image = parent().world().resource_manager().image("bar.png");
	bar.pos = pos + mark::vector<double>(0, -mark::module::size * 2);
	bar.type = mark::tick_context::bar_type::health;
	bar.percentage = m_cur_health / m_max_health;
	context.render(bar);

}

void mark::module::mortar::shoot(mark::vector<double> pos) {
	m_shoot = true;
}

auto mark::module::mortar::describe() const -> std::string {
	return "Mortar Module";
}
