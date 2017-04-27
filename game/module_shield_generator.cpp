#pragma once
#include "module_shield_generator.h"
#include "resource_manager.h"
#include "resource_image.h"
#include "sprite.h"
#include "tick_context.h"
#include <iostream>

mark::module::shield_generator::shield_generator(mark::resource::manager& resource_manager):
	base({ 2, 2 }, resource_manager.image("shield-generator.png")),
	m_im_generator(resource_manager.image("shield-generator.png")),
	m_model_shield(resource_manager, 128.f) {

}

void mark::module::shield_generator::tick(mark::tick_context& context) {
	const auto pos = this->pos();
	m_model_shield.tick(context, pos);
	context.sprites[0].push_back(mark::sprite(m_im_generator, pos, mark::module::size * 2.f, parent().rotation()));
}

auto mark::module::shield_generator::collides(mark::vector<double> pos, float radius) const -> bool {
	return mark::length(this->pos() - pos) < radius + 64.f;
}

void mark::module::shield_generator::damage(unsigned amount, mark::vector<double> pos) {
	m_model_shield.trigger(pos);
}

auto mark::module::shield_generator::describe() const -> std::string {
	return "Shield Generator Module";
}
