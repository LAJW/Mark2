#include "module_cargo.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"

mark::module::cargo::cargo(mark::resource::manager& resource_manager):
	mark::module::base({ 4, 2 }),
	m_im_body(resource_manager.image("cargo.png")),
	m_im_light(resource_manager.image("glare.png")),
	m_lfo(0.5f, static_cast<float>(resource_manager.random_double(0, 6))) {
}

void mark::module::cargo::tick(mark::tick_context& context) {
	m_lfo.tick(context.dt);
	auto pos = socket()->relative_pos();
	auto light_offset = mark::rotate(mark::vector<double>(24.f, 8.f), socket()->rotation());
	auto light_strength = static_cast<uint8_t>(255.f * (m_lfo.get() + 1.f) / 2.f);
	context.sprites[0].push_back(mark::sprite(m_im_body, pos, 64.f, socket()->rotation()));
	context.sprites[1].push_back(mark::sprite(m_im_light, pos + light_offset, 32.f, 0, 0, sf::Color(255, 200, 150, light_strength)));
	context.sprites[2].push_back(mark::sprite(m_im_light, pos + light_offset, 16.f, 0, 0, sf::Color(255, 255, 255, light_strength)));
}