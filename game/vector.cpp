#include <algorithm>
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
