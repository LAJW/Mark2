#pragma once
#include "module_turret.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"

mark::module::turret::turret(mark::resource::manager& resource_manager):
	base({ 2, 2 }),
	m_im_base(resource_manager.image("turret-base.png")),
	m_im_cannon(resource_manager.image("turret-cannon.png")) {

}

auto mark::module::turret::render() const -> std::vector<mark::sprite> {
	auto pos = socket()->relative_pos();
	return { mark::sprite(m_im_base, pos.x, pos.y, 32.f, socket()->rotation()) };
}

auto mark::module::turret::dead() const -> bool {
	return false;
}