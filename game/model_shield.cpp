#include "model_shield.h"
#include "resource_manager.h"
#include "sprite.h"
#include "stdafx.h"
#include "update_context.h"
#include <algorithm.h>
#include <particle.h>

mark::model::shield::shield(resource::manager& rm, float radius)
	: m_adsr(0, 255, 0.1f, 1)
	, m_lfo(1.f, rm.random(0.f, 1.f))
	, m_image_reflection(rm.image("shield-reaction.png"))
	, m_image_shield(rm.image("shield.png"))
	, m_image_shard(rm.image("particle.png"))
	, m_radius(radius)
{}

void mark::model::shield::update(
	update_context& context,
	vector<double> pos,
	bool active)
{
	// Shield "break" particle effect
	if (!active && m_active) {
		for (let i : range(double(m_radius))) {
			let offset =
				rotate(vector<double>(m_radius, 0.), i / m_radius * 360.);
			context.particles.push_back([&] {
				particle::info _;
				_.direction = float(atan(offset)) + context.random(-15.f, 15.f);
				_.image = m_image_shard;
				_.layer = 1;
				_.pos = offset + pos;
				_.size = 4.f;
				_.velocity = context.random(15.f, 30.f);
				_.lifespan = 1.f;
				return _;
			}());
		}
	} else if (active && !m_active) {
		// Set to .5 for faster recovery
		m_radius_multiplier = .5;
	}
	m_active = active;
	let dtf = static_cast<float>(context.dt);
	m_radius_multiplier = active
		? std::min(1.f, m_radius_multiplier + dtf)
		: std::max(0.f, m_radius_multiplier - dtf * 10.f);
	m_lfo.update(context.dt);
	m_adsr.update(context.dt);
	let shield_sprite_size = 2.f * m_radius * m_radius_multiplier;
	context.sprites[3].emplace_back([&] {
		let shield_opacity =
			static_cast<uint8_t>((m_lfo.get() * 0.5f + 0.5f) * 255.f);
		sprite _;
		_.image = m_image_shield;
		_.pos = pos;
		_.size = shield_sprite_size;
		_.color = { 150, 255, 255, shield_opacity };
		return _;
	}());

	context.sprites[3].emplace_back([&] {
		let reflection_opacity = static_cast<uint8_t>(m_adsr.get());
		sprite _;
		_.image = m_image_reflection;
		_.pos = pos;
		_.rotation = static_cast<float>(atan(m_trigger_pos - pos));
		_.size = shield_sprite_size;
		_.color = { 150, 255, 255, reflection_opacity };
		return _;
	}());
}

void mark::model::shield::trigger(vector<double> pos)
{
	m_adsr.trigger();
	m_trigger_pos = pos;
}
