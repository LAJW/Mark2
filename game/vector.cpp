#include "stdafx.h"
#include "vector.h"

auto mark::distance(const float alpha, const mark::vector<double> point) noexcept -> double {
	const auto a = std::tan(alpha / 180.f * static_cast<float>(M_PI));
	return std::abs(a * point.x + point.y) / std::sqrt(a * a + 1);
}

auto mark::get_line(const mark::vector<double> start, const mark::vector<double> end) noexcept -> mark::vector<double> {
	if (start.x == end.x) {
		if (start.y == end.y) {
			return { 0, start.y };
		} else {
			return { start.x, NAN };
		}
	} else {
		const auto a = (start.y - end.y) / (start.x - end.x);
		const auto b = start.y - a * start.x;
		return { a, b };
	}
}

auto mark::intersect(const mark::vector<double> line1, const mark::vector<double> line2) noexcept -> mark::vector<double> {
	if (isnan(line1.x) || isnan(line2.x)) {
		return { NAN, NAN };
	} else if (isnan(line1.y)) {
		if (isnan(line2.y)) {
			return { NAN, NAN };
		} else {
			const auto y = line1.x * line2.x + line2.y;
			return { line1.x, y };
		}
	} else if (isnan(line2.y)) {
		const auto y = line2.x * line1.x + line1.y;
		return { line2.x, y };
	} else if (line1.x == line2.x) {
		return { NAN, NAN };
	} else {
		const auto x = (line2.y - line1.y) / (line1.x - line2.x);
		const auto y = line1.x * x + line1.y;
		return { x, y };
	}
}

auto mark::intersect(const mark::segment_t s1, const mark::segment_t s2) noexcept -> mark::vector<double> {
	const auto line1 = mark::get_line(s1.first, s1.second);
	const auto line2 = mark::get_line(s2.first, s2.second);
	const auto intersection = mark::intersect(line1, line2);
	// lower x bound
	const auto lx = std::max(std::min(s1.first.x, s1.second.x), std::min(s2.first.x, s2.second.x));
	// upper x bound
	const auto ux = std::min(std::max(s1.first.x, s1.second.x), std::max(s2.first.x, s2.second.x));
	// lower y bound
	const auto ly = std::max(std::min(s1.first.y, s1.second.y), std::min(s2.first.y, s2.second.y));
	// upper y bound
	const auto uy = std::min(std::max(s1.first.y, s1.second.y), std::max(s2.first.y, s2.second.y));
	if (intersection.x >= lx && intersection.x <= ux && intersection.y >= ly && intersection.y <= uy) {
		return intersection;
	} else {
		return { NAN, NAN };
	}
}

auto mark::intersect(
	mark::segment_t segment,
	mark::vector<double> center,
	float radius) noexcept -> mark::vector<double> {
	const auto line = mark::get_line(segment.first, segment.second);
	const auto lx = std::min(segment.first.x, segment.second.x);
	const auto ux = std::max(segment.first.x, segment.second.x);
	const auto ly = std::min(segment.first.y, segment.second.y);
	const auto uy = std::max(segment.first.y, segment.second.y);
	if (!std::isnan(line.y)) {
		// everything but vertical line
		const auto c = line.x;
		const auto d = line.y;
		const auto a = center.x;
		const auto b = center.y;
		const auto r = static_cast<double>(radius);
		const auto e = d - b;
		const auto A = c * c + 1;
		const auto B = 2.0 * c * e - 2.0 * a;
		const auto C = a * a + e * e - r * r;
		const auto delta2 = B * B - 4 * A * C;
		if (delta2 >= 0) {
			const auto delta = std::sqrt(delta2);
			const auto x1 = (-B + delta) / (2.0 * A);
			const auto x2 = (-B - delta) / (2.0 * A);
			const auto y1 = c * x1 + d;
			const auto y2 = c * x2 + d;
			const auto p1 = mark::vector<double>(x1, y1);
			const auto p2 = mark::vector<double>(x2, y2);
			const auto length1 = mark::length(p1 - segment.first);
			const auto length2 = mark::length(p2 - segment.first);

			bool p1_in_range = p1.x >= lx && p1.x <= ux
				&& p1.y >= ly && p1.y <= uy;
			bool p2_in_range = p2.x >= lx && p2.x <= ux
				&& p2.y >= ly && p2.y <= uy;
			bool p1_closer = length1 < length2;
			if (p1_in_range && (p2_in_range && p1_closer || !p2_in_range)) {
				return p1;
			} else if (p2_in_range) {
				return p2;
			}
		}
	} else if (!std::isnan(line.x) && std::abs(line.x - center.x) <= radius) {
		// vertical line
		const auto a = line.x;
		const auto b = line.y;
		const auto r = static_cast<double>(radius);
		const auto A = center.x - line.x;
		const auto B = std::sqrt(A * A + r * r);
		const auto y1 = center.y - B;
		const auto y2 = center.y + B;
		const auto length1 = std::abs(y1 - segment.first.y);
		const auto length2 = std::abs(y2 - segment.first.y);
		const auto y1_in_range = y1 >= ly && y1 <= uy;
		const auto y2_in_range = y2 >= ly && y2 <= uy;

		if (y1_in_range && y2_in_range && length1 < length2 || !y2_in_range) {
			return { line.x, y1 };
		} else if (y2_in_range) {
			return { line.x, y2 };
		}
	}
	return { NAN, NAN };
}

// Calculate new rotation for an entity based on angular velocity, lookat direction, etc.
auto mark::turn(mark::vector<double> new_direction, float current_rotation, float angular_velocity, double dt) -> float {
	const auto turn_direction = mark::sgn(mark::atan(mark::rotate(new_direction, -current_rotation)));
	const auto rot_step = static_cast<float>(turn_direction  * angular_velocity * dt);
	if (std::abs(mark::atan(mark::rotate(new_direction, -current_rotation))) < angular_velocity * dt) {
		return static_cast<float>(mark::atan(new_direction));
	} else {
		return current_rotation + rot_step;
	}
}
