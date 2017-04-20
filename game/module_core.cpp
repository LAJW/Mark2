#pragma once
#include "module_core.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"

mark::module::core::core(mark::resource::manager& resource_manager):
	base({ 2, 2 }, resource_manager.image("core.png")),
	m_image(resource_manager.image("core.png")) {
	
}

void mark::module::core::tick(mark::tick_context& context) {
	auto pos = socket()->relative_pos();
	context.sprites[0].push_back(mark::sprite(m_image, pos.x, pos.y, 32.f, socket()->rotation()));
}

auto mark::module::core::dead() const -> bool {
	return m_health <= 0;
}