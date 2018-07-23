#pragma once
#include "adsr.h"
#include "lfo.h"
#include "stdafx.h"
#include "vector.h"

namespace mark {
struct update_context;
namespace resource {
class manager;
class image;
} // namespace resource
namespace model {
class shield
{
public:
	shield(
		resource::manager& resource_manager,
		mark::random& random,
		float radius);
	void update(update_context& context, vd pos, bool active);
	void trigger(vd pos);

private:
	lfo m_lfo;
	adsr m_adsr;
	const resource::image_ptr m_image_shield;
	const resource::image_ptr m_image_reflection;
	// Texture used for the "break" effect
	const resource::image_ptr m_image_shard;
	// State of the "active/break" animation.
	// 1 - shield is fully expanded
	// 0 - shield has collapsed
	float m_radius_multiplier = 1.f;
	const float m_radius;
	vd m_trigger_pos;
	bool m_active = true;
};
} // namespace model
} // namespace mark
