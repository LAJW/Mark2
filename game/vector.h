#pragma once
#define _USE_MATH_DEFINES
#include <SFML/System/Vector2.hpp>
#include <math.h>

namespace mark {
	template<typename T>
	using vector = sf::Vector2<T>;
	template<typename T, typename U>
	auto rotate(T vector, U angle) {
		auto alpha = angle / static_cast<U>(180.0) * static_cast<U>(M_PI);
		auto cs = std::cos(alpha);
		auto sn = std::sin(alpha);
		auto px = vector.x * cs - vector.y * sn;
		auto py = vector.x * sn + vector.y * cs;
		return T(px, py);
	}
	template<typename T>
	auto length(mark::vector<T> vector) {
		return std::sqrt(vector.x * vector.x + vector.y * vectory.y);
	}
}