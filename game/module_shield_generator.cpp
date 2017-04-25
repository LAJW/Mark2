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
	m_im_shield(resource_manager.image("shield.png")),
	m_im_glare(resource_manager.image("glare.png")),
	m_light_lfo(1.f, static_cast<float>(resource_manager.random_double(0, 1))),
	m_shield_lfo(1.1f, static_cast<float>(resource_manager.random_double(0, 1))) {

}

void mark::module::shield_generator::tick(mark::tick_context& context) {
	m_shield_lfo.tick(context.dt);
	m_light_lfo.tick(context.dt);
	const auto pos = socket()->relative_pos();
	const auto light_intensity = static_cast<uint8_t>(255.f * (m_light_lfo.get() + 1.f) / 2.f);
	const auto shield_intensity = static_cast<uint8_t>(255.f * (m_shield_lfo.get() + 1.f) / 2.f);
	context.sprites[0].push_back(mark::sprite(m_im_generator, pos, 32.f, socket()->rotation()));
	context.sprites[1].push_back(mark::sprite(m_im_shield, pos, 128.f, 0.0f, 0, sf::Color(255, 255, 255, shield_intensity)));
	context.sprites[2].push_back(mark::sprite(m_im_glare, pos, 64.f, 0.0f, 0, sf::Color(0, 255, 255, light_intensity)));
	context.sprites[2].push_back(mark::sprite(m_im_glare, pos, 16.f, 0.0f, 0, sf::Color(255, 255, 255, light_intensity)));
}

auto mark::module::shield_generator::collides(mark::vector<double> pos, float radius) const -> bool {
	return mark::length(this->socket()->relative_pos() - pos) < radius + 64.f;
}