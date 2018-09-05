#pragma once
#include <stdafx.h>

/// \file unit::modular implementation functions

namespace mark {
namespace unit {
namespace impl {

struct should_accelerate_info final
{
	double rotation;
	double target_angle;
	double angular_velocity;
	double angular_acceleration;
};

struct rotation_and_angular_velocity_info final
{
	double rotation;
	double angular_velocity;
	double angular_acceleration;
	double dt;
	vd target; // relative target position
};

[[nodiscard]] double limit_angle(double angle);

[[nodiscard]] bool
should_accelerate(const should_accelerate_info& info);

[[nodiscard]] std::tuple<double, double>
rotation_and_angular_velocity(const rotation_and_angular_velocity_info& info);

} // namespace impl
} // namespace unit
} // namespace mark
