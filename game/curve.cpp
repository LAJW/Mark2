#include "stdafx.h"
#include "curve.h"
#include "exception.h"

auto mark::curve::serialize(mark::curve::ptr ptr) -> std::string {
	if (ptr == mark::curve::flat) {
		return "flat";
	} else if (ptr == mark::curve::invert) {
		return "invert";
	} else if (ptr == mark::curve::linear) {
		return "linear";
	} else if (ptr == mark::curve::sin) {
		return "sin";
	} else {
		throw mark::exception("BAD_CURVE");
	}
}

auto mark::curve::deserialize(const std::string& str) -> mark::curve::ptr {
	if (str == "flat") {
		return mark::curve::flat;
	} else if (str == "invert") {
		return mark::curve::invert;
	} else if (str == "linear") {
		return mark::curve::linear;
	} else if (str == "sin") {
		return mark::curve::sin;
	} else {
		throw mark::exception("BAD_CURVE");
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
