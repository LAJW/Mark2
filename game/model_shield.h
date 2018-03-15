#pragma once
#include "adsr.h"
#include "lfo.h"
#include "stdafx.h"
#include "vector.h"

namespace mark {
struct tick_context;
namespace resource {
class manager;
class image;
} // namespace resource
namespace model {
class shield {
public:
	shield(resource::manager& resource_manager, float radius);
	void tick(tick_context& context, vector<double> pos);
	void trigger(vector<double> pos);

private:
	lfo m_lfo;
	adsr m_adsr;
	const std::shared_ptr<const resource::image> m_image_shield;
	const std::shared_ptr<const resource::image> m_image_reflection;
	const float m_radius;
	vector<double> m_trigger_pos;
};
} // namespace model
} // namespace mark