#pragma once
#include "module_core.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"

mark::module::core::core(mark::resource::manager& resource_manager)
	:base({ 2, 2 }), m_image(resource_manager.image("core.png")) {
	
}

auto mark::module::core::render() const -> std::vector<mark::sprite> {
	auto pos = socket()->relative_pos();
	return { mark::sprite(m_image, pos.x, pos.y, 32.f, socket()->rotation()) };
}

auto mark::module::core::dead() const -> bool {
	return m_health <= 0;
}