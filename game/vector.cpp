#include "stdafx.h"
#include "vector.h"

auto mark::distance(const float alpha, const vector<double> point) noexcept -> double {
	const auto a = std::tan(alpha / 180.f * static_cast<float>(M_PI));
	return std::abs(a * point.x + point.y) / std::sqrt(a * a + 1);
}

auto mark::get_line(
	const vector<double> start, const vector<double> end) noexcept
	-> std::variant<vector<double>, double>
{
	if (start.x == end.x) {
		if (start.y == end.y) {
			return vector<double>(0, start.y);
		}
		return start.x;
	}
	const auto a = (start.y - end.y) / (start.x - end.x);
	const auto b = start.y - a * start.x;
	return vector<double>(a, b);
}

auto mark::intersect(
	const std::variant<vector<double>, double> line1,
	const std::variant<vector<double>, double> line2) noexcept
	-> std::optional<vector<double>>
{
	const auto intersect_with_X = [](vector<double> line, double x) {
		const auto y = x * line.x + line.y;
		return vector<double>{ x, y };
	};
	if (const auto x = std::get_if<double>(&line1)) {
		if (const auto line = std::get_if<vector<double>>(&line2)) {
			return intersect_with_X(*line, *x);
		}
		return { };
	}
	if (const auto x = std::get_if<double>(&line2)) {
		return intersect_with_X(std::get<vector<double>>(line1), *x);
	}
	const auto l1 = std::get<vector<double>>(line1);
	const auto l2 = std::get<vector<double>>(line2);
	if (l1.x == l2.x) {
		return { };
	}
	const auto x = (l2.y - l1.y) / (l1.x - l2.x);
	const auto y = l1.x * x + l1.y;
	return { { x, y } };
}

auto mark::intersect(const segment_t& s1, const segment_t& s2) noexcept
	-> std::optional<vector<double>>
{
	const auto line1 = get_line(s1.first, s1.second);
	const auto line2 = get_line(s2.first, s2.second);
	const auto intersection = intersect(line1, line2);
	if (!intersection) {
		return { };
	}
	const auto lower_x_bound = std::max(
		std::min(s1.first.x, s1.second.x), std::min(s2.first.x, s2.second.x));
	const auto upper_x_bound = std::min(
		std::max(s1.first.x, s1.second.x), std::max(s2.first.x, s2.second.x));
	const auto lower_y_bound = std::max(
		std::min(s1.first.y, s1.second.y), std::min(s2.first.y, s2.second.y));
	const auto upper_y_bound = std::min(
		std::max(s1.first.y, s1.second.y), std::max(s2.first.y, s2.second.y));
	// Margin for error in a floating point operation
	constexpr const auto flp_margin = 0.5;
	// Margins are required here, because x >= C && x <= C evaluates to x == C
	// Floating point comparison is imprecise, so an error margin is required
	// Adding this makes intersection with horizontal lines work
	if (intersection->x >= lower_x_bound - flp_margin
		&& intersection->x <= upper_x_bound + flp_margin
		&& intersection->y >= lower_y_bound - flp_margin
		&& intersection->y <= upper_y_bound + flp_margin) {
		return intersection;
	}
	return { };
}

auto mark::intersect(
	segment_t segment, const vector<double>& center, double radius) noexcept
	-> std::optional<vector<double>>
{
	const auto line = get_line(segment.first, segment.second);
	const auto lx = std::min(segment.first.x, segment.second.x);
	const auto ux = std::max(segment.first.x, segment.second.x);
	const auto ly = std::min(segment.first.y, segment.second.y);
	const auto uy = std::max(segment.first.y, segment.second.y);
	if (const auto line_nv = std::get_if<vector<double>>(&line)) {
		// everything but vertical line
		const auto c = line_nv->x;
		const auto d = line_nv->y;
		const auto [A, B, delta2] = [&] {
			const auto a = center.x;
			const auto b = center.y;
			const auto r = static_cast<double>(radius);
			const auto e = d - b;
			const auto A = c * c + 1;
			const auto B = 2.0 * c * e - 2.0 * a;
			const auto C = a * a + e * e - r * r;
			const auto delta2 = B * B - 4 * A * C;
			return std::make_tuple(A, B, delta2);
		}();
		if (delta2 <= 0)
			return { };
		const auto[p1, p2] = [&] {
			const auto delta = std::sqrt(delta2);
			const auto x1 = (-B + delta) / (2.0 * A);
			const auto x2 = (-B - delta) / (2.0 * A);
			const auto y1 = c * x1 + d;
			const auto y2 = c * x2 + d;
			return std::make_pair(vector<double>(x1, y1), vector<double>(x2, y2));
		}();
		const bool p1_in_range = p1.x >= lx && p1.x <= ux
			&& p1.y >= ly && p1.y <= uy;
		const bool p2_in_range = p2.x >= lx && p2.x <= ux
			&& p2.y >= ly && p2.y <= uy;
		const auto p1_closer = [&] {
			const auto length1 = length(p1 - segment.first);
			const auto length2 = length(p2 - segment.first);
			return length1 < length2;
		}();
		if (p1_in_range && (p2_in_range && p1_closer || !p2_in_range)) {
			return p1;
		} else if (p2_in_range) {
			return p2;
		}
	} else {
		// vertical line
		const auto x = std::get<double>(line);
		if (std::abs(x - center.x) <= radius) {
			const auto[y1, y2] = [&] {
				const auto r = static_cast<double>(radius);
				const auto A = center.x - x;
				const auto B = std::sqrt(A * A + r * r);
				const auto y1 = center.y - B;
				const auto y2 = center.y + B;
				return std::make_pair(y1, y2);
			}();
			const auto length1 = std::abs(y1 - segment.first.y);
			const auto length2 = std::abs(y2 - segment.first.y);
			const auto y1_in_range = y1 >= ly && y1 <= uy;
			const auto y2_in_range = y2 >= ly && y2 <= uy;

			if (y1_in_range && y2_in_range && length1 < length2 || !y2_in_range) {
				return { { x, y1 } };
			} else if (y2_in_range) {
				return { { x, y2 } };
			}
		}
	}
	return { };
}

// Calculate new rotation for an entity based on angular velocity, lookat direction, etc.
auto mark::turn(
	vector<double> new_direction,
	float current_rotation,
	float angular_velocity,
	double dt) -> float
{
	if (std::abs(atan(rotate(new_direction, -current_rotation))) < angular_velocity * dt) {
		return static_cast<float>(atan(new_direction));
	}
	const auto turn_direction = sgn(atan(rotate(new_direction, -current_rotation)));
	const auto rot_step = static_cast<float>(turn_direction  * angular_velocity * dt);
	return current_rotation + rot_step;
}
