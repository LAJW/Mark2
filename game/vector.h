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
		return std::sqrt(vector.x * vector.x + vector.y * vector.y);
	}
	template<typename T>
	auto normalize(mark::vector<T> vector) {
		const auto length = mark::length(vector);
		if (length) {
			return vector / length;
		} else {
			return mark::vector<T>(0, 0);
		}
	}
	// get vector's arcus tangens in degrees
	template<typename T>
	auto atan(const mark::vector<T>& vector) {
		return std::atan2(vector.y, vector.x) / static_cast<float>(M_PI) * 180.f;
	}
	template<typename T>
	T sgn(T val) {
		if (val > 0) {
			return 1;
		} else if (val < 0) {
			return -1;
		} else {
			return 0;
		}
	}
}