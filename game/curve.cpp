#include "stdafx.h"
#include "curve.h"
#include "exception.h"

auto mark::curve::serialise(curve::ptr ptr) -> std::string {
	if (ptr == curve::flat) {
		return "flat";
	} else if (ptr == curve::invert) {
		return "invert";
	} else if (ptr == curve::linear) {
		return "linear";
	} else if (ptr == curve::sin) {
		return "sin";
	} else {
		throw exception("BAD_CURVE");
	}
}

auto mark::curve::deserialise(const std::string& str) -> curve::ptr {
	if (str == "flat") {
		return curve::flat;
	} else if (str == "invert") {
		return curve::invert;
	} else if (str == "linear") {
		return curve::linear;
	} else if (str == "sin") {
		return curve::sin;
	} else {
		throw exception("BAD_CURVE");
	}
}

float mark::curve::flat(float) {
	return 1.f;
}

float mark::curve::linear(float x) {
	return std::max(std::min(1.f, x), 0.f);
}

float mark::curve::invert(float x) {
	return linear(1 - x);
}

float mark::curve::sin(float x) {
	return std::sin(x * (float)M_PI * 2.f) * 0.5f + 0.5f;
}
