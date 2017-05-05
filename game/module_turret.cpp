#pragma once
#include "module_turret.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"
#include "unit_projectile.h"
#include "unit_modular.h"
#include "world.h"

mark::module::turret::turret(mark::resource::manager& resource_manager):
	base({ 2, 2 }, resource_manager.image("turret-base.png")),
	m_im_base(resource_manager.image("turret-base.png")),
	m_im_cannon(resource_manager.image("turret-cannon.png")),
	m_adsr(0.1f, 8.f, 0.1f, 0.8f) {

}

void mark::module::turret::tick(mark::tick_context& context) {
	m_adsr.tick(context.dt);
	auto pos = this->pos();
	auto angle = static_cast<float>(mark::atan(m_target - pos));
	if (m_cur_cooldown >= 0) {
		m_cur_cooldown -= static_cast<float>(context.dt);
	} else if (m_shoot) {
		m_cur_cooldown = 0.5f;
		m_adsr.trigger();
		mark::unit::projectile::attributes attr;
		attr.world = &parent().world();
		attr.guide = std::dynamic_pointer_cast<mark::unit::modular>(parent().shared_from_this());
		attr.pos = pos;
		attr.rotation = angle;
		attr.velocity = 500.f;
		attr.aoe_radius = 300.f;
		attr.team = parent().team();
		attr.piercing = 0;
		context.units.emplace_back(std::make_shared<mark::unit::projectile>(attr));
	}
	m_shoot = false;
	context.sprites[1].push_back(mark::sprite(m_im_cannon, pos - mark::rotate(mark::vector<double>(m_adsr.get() - 16.0, 0.0), angle), 32.f, angle));
	context.sprites[0].push_back(mark::sprite(m_im_base, pos.x, pos.y, 32.f, parent().rotation()));
	context.render_bar(
		parent().world().resource_manager().image("bar.png"),
		pos + mark::vector<double>(0, -mark::module::size * 2),
		mark::tick_context::bar_type::health,
		m_cur_health / m_max_health);

}

void mark::module::turret::target(mark::vector<double> pos) {
	m_target = pos;
}

void mark::module::turret::shoot(mark::vector<double> pos) {
	m_target = pos;
	m_shoot = true;
}

auto mark::module::turret::describe() const -> std::string {
	return "Turret Module";
}
