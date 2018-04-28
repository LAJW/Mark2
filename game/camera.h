#pragma once
#include <adsr.h>
#include <lfo.h>
#include <stdafx.h>

namespace mark {
class camera final
{
public:
	camera() = default;
	camera(const YAML::Node& node);
	// Set target unit
	void target(shared_ptr<unit::base> target);
	auto target() -> shared_ptr<unit::base>;
	auto target() const -> shared_ptr<const unit::base>;
	void update(double dt);
	auto pos() const -> vd;
	// Trigger camera shake
	void trigger();
	void serialize(YAML::Emitter& out) const;

private:
	vd m_pos;
	double m_velocity = 0.;
	shared_ptr<unit::base> m_target;
	lfo m_x_lfo = lfo(6.f, .5f);
	lfo m_y_lfo = lfo(10.f, .0f);
	adsr m_adsr = adsr(0, 1, .5f, .3f);
	double m_a = 0.;
	vd m_prev_target_pos;
};
} // namespace mark
