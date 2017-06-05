#pragma once
#include "stdafx.h"

namespace mark {
	namespace curve {
		typedef float(*ptr)(float);
		auto deserialize(const std::string&) -> ptr;
		auto serialize(curve::ptr) -> std::string;
		auto flat(float) -> float;
		auto linear(float x) -> float;
		auto invert(float x) -> float;
		auto sin(float x) -> float;
	}
}