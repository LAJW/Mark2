#include "modular.h"

namespace mark {
namespace unit {
namespace impl {

/// Limit angle to (-180, 180] degree rotation
double limit_angle(double angle)
{
	if (angle > 180.) {
		return angle - 360.;
	} else if (angle <= -180.) {
		return angle + 360.;
	} else {
		return angle;
	}
}

double should_accelerate(const should_accelerate_info& info)
{
	let[rotation, target_angle, angular_velocity, angular_acceleration] = info;
	let remaining_angle = abs(limit_angle(rotation - target_angle));
	let angle = pow(angular_velocity, 2.) / angular_acceleration
		- .5 * pow(angular_velocity, 3.) / pow(angular_acceleration, 2.);
	return std::abs(limit_angle(angle)) <= remaining_angle;
}

std::tuple<double, double>
rotation_and_angular_velocity(const rotation_and_angular_velocity_info& info)
{
	let[rotation, angular_velocity, angular_acceleration, dt, target] = info;
	let target_angle = atan(target);
	let should_accelerate = impl::should_accelerate([&] {
		should_accelerate_info _;
		_.rotation = rotation;
		_.angular_acceleration = angular_acceleration;
		_.angular_velocity = angular_velocity;
		_.target_angle = target_angle;
		return _;
	}());
	auto new_angular_velocity = [&] {
		if (should_accelerate) {
			return angular_velocity + angular_acceleration * dt;
		}
		let diff = abs(rotation - target_angle);
		let remaining_angle = std::abs(diff <= 180.) ? diff : 360. - diff;
		if (remaining_angle < .5) {
			return angular_velocity;
		}
		let deceleration = pow(angular_velocity, 2.) / remaining_angle;
		return abs(angular_velocity - deceleration * dt);
	}();
	auto new_rotation = limit_angle(turn(
		target,
		gsl::narrow_cast<float>(rotation),
		gsl::narrow_cast<float>(new_angular_velocity),
		dt));
	if (new_rotation <= target_angle + .5
		&& new_rotation >= target_angle - .5) {
		return { target_angle, 0.0 };
	}
	Ensures(new_rotation <= 180.0 && new_rotation >= -180.0);
	Ensures(new_angular_velocity >= 0.);
	return { new_rotation, new_angular_velocity };
}

} // namespace impl
} // namespace unit
} // namespace mark
