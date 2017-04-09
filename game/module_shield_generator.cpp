#pragma once
#include "module_shield_generator.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"

mark::module::shield_generator::shield_generator(mark::resource::manager& resource_manager):
	base({ 2, 2 }),
	m_im_generator(resource_manager.image("shield-generator.png")),
	m_im_shield(resource_manager.image("shield.png")) {

}

auto mark::module::shield_generator::render() const -> std::vector<mark::sprite> {
	auto pos = socket()->relative_pos();
	return {
		mark::sprite(m_im_generator, pos.x, pos.y, 32.f, socket()->rotation()),
		mark::sprite(m_im_shield, pos.x, pos.y, 128.f)
	};
}