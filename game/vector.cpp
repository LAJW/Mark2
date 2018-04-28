#include "stdafx.h"
#include "vector.h"

auto mark::distance(const float alpha, const vd point) noexcept
	-> double
{
	let a = std::tan(alpha / 180.f * static_cast<float>(M_PI));
	return std::abs(a * point.x + point.y) / std::sqrt(a * a + 1);
}

auto mark::get_line(
	const vd start,
	const vd end) noexcept -> std::variant<vd, double>
{
	if (start.x == end.x) {
		if (start.y == end.y) {
			return vd(0, start.y);
		}
		return start.x;
	}
	let a = (start.y - end.y) / (start.x - end.x);
	let b = start.y - a * start.x;
	return vd(a, b);
}

auto mark::intersect(
	const std::variant<vd, double> line1,
	const std::variant<vd, double> line2) noexcept
	-> std::optional<vd>
{
	let intersect_with_X = [](vd line, double x) {
		let y = x * line.x + line.y;
		return vd{ x, y };
	};
	if (let x = std::get_if<double>(&line1)) {
		if (let line = std::get_if<vd>(&line2)) {
			return intersect_with_X(*line, *x);
		}
		return {};
	}
	if (let x = std::get_if<double>(&line2)) {
		return intersect_with_X(std::get<vd>(line1), *x);
	}
	let l1 = std::get<vd>(line1);
	let l2 = std::get<vd>(line2);
	if (l1.x == l2.x) {
		return {};
	}
	let x = (l2.y - l1.y) / (l1.x - l2.x);
	let y = l1.x * x + l1.y;
	return { { x, y } };
}

auto mark::intersect(const segment_t& s1, const segment_t& s2) noexcept
	-> std::optional<vd>
{
	let line1 = get_line(s1.first, s1.second);
	let line2 = get_line(s2.first, s2.second);
	let intersection = intersect(line1, line2);
	if (!intersection) {
		return {};
	}
	let lower_x_bound = std::max(
		std::min(s1.first.x, s1.second.x), std::min(s2.first.x, s2.second.x));
	let upper_x_bound = std::min(
		std::max(s1.first.x, s1.second.x), std::max(s2.first.x, s2.second.x));
	let lower_y_bound = std::max(
		std::min(s1.first.y, s1.second.y), std::min(s2.first.y, s2.second.y));
	let upper_y_bound = std::min(
		std::max(s1.first.y, s1.second.y), std::max(s2.first.y, s2.second.y));
	// Margin for error in a floating point operation
	constexpr let flp_margin = 0.5;
	// Margins are required here, because x >= C && x <= C evaluates to x == C
	// Floating point comparison is imprecise, so an error margin is required
	// Adding this makes intersection with horizontal lines work
	if (intersection->x >= lower_x_bound - flp_margin
		&& intersection->x <= upper_x_bound + flp_margin
		&& intersection->y >= lower_y_bound - flp_margin
		&& intersection->y <= upper_y_bound + flp_margin) {
		return intersection;
	}
	return {};
}

auto mark::intersect(
	segment_t segment,
	const vd& center,
	double radius) noexcept -> std::optional<vd>
{
	let line = get_line(segment.first, segment.second);
	let lx = std::min(segment.first.x, segment.second.x);
	let ux = std::max(segment.first.x, segment.second.x);
	let ly = std::min(segment.first.y, segment.second.y);
	let uy = std::max(segment.first.y, segment.second.y);
	if (let line_nv = std::get_if<vd>(&line)) {
		// everything but vertical line
		let c = line_nv->x;
		let d = line_nv->y;
		let[A, B, delta2] = [&] {
			let a = center.x;
			let b = center.y;
			let r = static_cast<double>(radius);
			let e = d - b;
			let A = c * c + 1;
			let B = 2.0 * c * e - 2.0 * a;
			let C = a * a + e * e - r * r;
			let delta2 = B * B - 4 * A * C;
			return std::make_tuple(A, B, delta2);
		}();
		if (delta2 <= 0)
			return {};
		let[p1, p2] = [&, delta2 = delta2, B = B, A = A] {
			let delta = std::sqrt(delta2);
			let x1 = (-B + delta) / (2.0 * A);
			let x2 = (-B - delta) / (2.0 * A);
			let y1 = c * x1 + d;
			let y2 = c * x2 + d;
			return std::make_pair(
				vd(x1, y1), vd(x2, y2));
		}();
		const bool p1_in_range =
			p1.x >= lx && p1.x <= ux && p1.y >= ly && p1.y <= uy;
		const bool p2_in_range =
			p2.x >= lx && p2.x <= ux && p2.y >= ly && p2.y <= uy;
		let p1_closer = [&, p1 = p1, p2 = p2] {
			let length1 = length(p1 - segment.first);
			let length2 = length(p2 - segment.first);
			return length1 < length2;
		}();
		if (p1_in_range && ((p2_in_range && p1_closer) || !p2_in_range)) {
			return p1;
		} else if (p2_in_range) {
			return p2;
		}
	} else {
		// vertical line
		let x = std::get<double>(line);
		if (std::abs(x - center.x) <= radius) {
			let[y1, y2] = [&] {
				let r = static_cast<double>(radius);
				let A = center.x - x;
				let B = std::sqrt(A * A + r * r);
				let y1 = center.y - B;
				let y2 = center.y + B;
				return std::make_pair(y1, y2);
			}();
			let length1 = std::abs(y1 - segment.first.y);
			let length2 = std::abs(y2 - segment.first.y);
			let y1_in_range = y1 >= ly && y1 <= uy;
			let y2_in_range = y2 >= ly && y2 <= uy;

			if ((y1_in_range && y2_in_range && length1 < length2)
				|| !y2_in_range) {
				return { { x, y1 } };
			} else if (y2_in_range) {
				return { { x, y2 } };
			}
		}
	}
	return {};
}

// Calculate new rotation for an entity based on angular velocity, lookat
// direction, etc.
auto mark::turn(
	vd new_direction,
	float current_rotation,
	float angular_velocity,
	double dt) -> float
{
	if (std::abs(atan(rotate(new_direction, -current_rotation)))
		< angular_velocity * dt) {
		return static_cast<float>(atan(new_direction));
	}
	let turn_direction = sgn(atan(rotate(new_direction, -current_rotation)));
	let rot_step = static_cast<float>(turn_direction * angular_velocity * dt);
	return current_rotation + rot_step;
}
