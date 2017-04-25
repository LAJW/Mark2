#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "model_shield.h"

mark::model::shield::shield(mark::resource::manager& resource_manager, float radius):
	m_adsr(0, 255, 0.1f, 1),
	m_lfo(1.f, 0.f),
	m_image_reflection(resource_manager.image("shield-reaction.png")),
	m_image_shield(resource_manager.image("shield.png")),
	m_radius(radius) { }

void mark::model::shield::tick(mark::tick_context & context, mark::vector<double> pos) {
	m_lfo.tick(context.dt);
	m_adsr.tick(context.dt);
	const auto shield_opacity = static_cast<uint8_t>((m_lfo.get() * 0.5f + 0.5f) * 255.f);
	const auto reflection_opacity = static_cast<uint8_t>(m_adsr.get());
	const auto rotation = static_cast<float>(mark::atan(m_trigger_pos - pos));
	context.sprites[1].push_back(mark::sprite(m_image_shield, pos, m_radius, 0, 0, sf::Color(150, 255, 255, shield_opacity)));
	context.sprites[1].push_back(mark::sprite(m_image_reflection, pos, m_radius, rotation, 0, sf::Color(150, 255, 255, reflection_opacity)));
}

void mark::model::shield::trigger(mark::vector<double> pos) {
	m_adsr.trigger();
	m_trigger_pos = pos;
}
