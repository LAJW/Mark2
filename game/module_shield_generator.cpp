#pragma once
#include "module_shield_generator.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"

mark::module::shield_generator::shield_generator(mark::resource::manager& resource_manager):
	base({ 2, 2 }),
	m_im_generator(resource_manager.image("shield-generator.png")),
	m_im_shield(resource_manager.image("shield.png")) {

}

void mark::module::shield_generator::tick(mark::tick_context& context) {
	auto pos = socket()->relative_pos();
	context.sprites[0].push_back(mark::sprite(m_im_generator, pos.x, pos.y, 32.f, socket()->rotation()));
	context.sprites[1].push_back(mark::sprite(m_im_shield, pos.x, pos.y, 128.f));
}