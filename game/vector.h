#pragma once
#include "stdafx.h"
#include <SFML/System/Vector2.hpp>

namespace mark {
template <typename T>
using vector = sf::Vector2<T>;

using vi32 = vector<int32_t>;
using vi8 = vector<int8_t>;
using vu32 = vector<uint32_t>;
using vd = vector<double>;

struct collide_result
{
	vd pos;
	float reflected_angle = 0.f;
};

template <typename T>
auto min(vector<T> t, vector<T> u)
{
	return vector<T>{ std::min(t.x, u.x), std::min(t.y, u.y) };
}

template <typename T>
auto max(vector<T> t, vector<T> u)
{
	return vector<T>{ std::max(t.x, u.x), std::max(t.y, u.y) };
}

template <typename T, typename U>
auto rotate(T vector, U angle)
{
	auto alpha = angle / static_cast<U>(180.0) * static_cast<U>(M_PI);
	auto cs = std::cos(alpha);
	auto sn = std::sin(alpha);
	auto px = vector.x * cs - vector.y * sn;
	auto py = vector.x * sn + vector.y * cs;
	return T(px, py);
}
template <typename T>
auto length(vector<T> vector)
{
	return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}
template <typename T>
auto normalize(vector<T> vector)
{
	let length = mark::length(vector);
	if (length) {
		return vector / length;
	} else {
		return mark::vector<T>(0, 0);
	}
}
// get vector's arcus tangens in degrees
template <typename T>
float atan(const vector<T>& vector)
{
	return static_cast<float>(std::atan2(vector.y, vector.x))
		/ static_cast<float>(M_PI) * 180.f;
}
// convert [i, width] into vector
template <typename T>
auto modulo_vector(T i, T width)
{
	return vector<T>(i % width, i / width);
}
template <typename T>
T sgn(T val)
{
	if (val > 0) {
		return 1;
	} else if (val < 0) {
		return -1;
	} else {
		return 0;
	}
}

template <typename T>
inline auto vmap(const vector<T> vector, T (*proc)(T))
{
	return mark::vector<T>(proc(vector.x), proc(vector.y));
}
inline auto round(const vd in) noexcept { return vi32(vmap(in, std::round)); }
inline auto floor(const vd in) noexcept { return vi32(vmap(in, std::floor)); }
// distance between point and a line: tan(alpha) + 0
auto distance(float alpha, vd point) noexcept -> double;

// Given 2 points, return [ a,b ] from y = ax + b
// If vertical line, only double is returned
auto get_line(vd start, vd end) noexcept -> std::variant<vd, double>;

// given 2 lines, find intersection point
auto intersect(
	std::variant<vd, double> line1,
	std::variant<vd, double> line2) noexcept -> std::optional<vd>;

// given 2 segments, find intersecting point
using segment_t = std::pair<vd, vd>;
auto intersect(const segment_t&, const segment_t&) noexcept
	-> std::optional<vd>;

// given segment and a circle, get nearerst intersection
auto intersect(segment_t, const vd& center, double radius) noexcept
	-> std::optional<vd>;

// Calculate new rotation for an entity based on angular velocity, lookat
// direction, etc.
auto turn(
	vd new_direction,
	float current_rotation,
	float angular_velocity,
	double dt) -> float;
} // namespace mark
