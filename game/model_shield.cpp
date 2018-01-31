#include "stdafx.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "model_shield.h"

mark::model::shield::shield(resource::manager& rm, float radius)
	: m_adsr(0, 255, 0.1f, 1)
	, m_lfo(1.f, rm.random(0.f, 1.f))
	, m_image_reflection(rm.image("shield-reaction.png"))
	, m_image_shield(rm.image("shield.png"))
	, m_radius(radius) { }

void mark::model::shield::tick(tick_context& context, vector<double> pos) {
	m_lfo.tick(context.dt);
	m_adsr.tick(context.dt);
	context.sprites[1].emplace_back([&] {
		let shield_opacity = static_cast<uint8_t>((m_lfo.get() * 0.5f + 0.5f) * 255.f);
		sprite _;
		_.image = m_image_shield;
		_.pos = pos;
		_.size = m_radius;
		_.color = { 150, 255, 255, shield_opacity };
		return _;
	}());
	
	context.sprites[1].emplace_back([&] {
		let reflection_opacity = static_cast<uint8_t>(m_adsr.get());
		sprite _;
		_.image = m_image_reflection;
		_.pos = pos;
		_.rotation = static_cast<float>(atan(m_trigger_pos - pos));
		_.size = m_radius;
		_.color = { 150, 255, 255, reflection_opacity };
		return _;
	}());
}

void mark::model::shield::trigger(vector<double> pos) {
	m_adsr.trigger();
	m_trigger_pos = pos;
}
